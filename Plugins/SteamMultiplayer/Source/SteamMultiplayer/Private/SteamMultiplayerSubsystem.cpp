// SteamMultiplayerSubsystem.cpp

#include "SteamMultiplayerSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Online/OnlineSessionNames.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"
#include "GameMapsSettings.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY(LogSteamMultiplayer);

const FName USteamMultiplayerSubsystem::SETTING_SERVER_NAME(TEXT("SERVER_NAME"));

USteamMultiplayerSubsystem::USteamMultiplayerSubsystem()
{
}

void USteamMultiplayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (!OnlineSubsystem)
	{
		UE_LOG(LogSteamMultiplayer, Warning, TEXT("SteamMultiplayerSubsystem: No OnlineSubsystem found."));
		return;
	}

	CachedSubsystemName = OnlineSubsystem->GetSubsystemName();
	UE_LOG(LogSteamMultiplayer, Log, TEXT("SteamMultiplayerSubsystem: OnlineSubsystem = %s"),
	       *CachedSubsystemName.ToString());

	SessionInterface = OnlineSubsystem->GetSessionInterface();
	ExternalUI = OnlineSubsystem->GetExternalUIInterface();

	if (SessionInterface.IsValid())
	{
		// Standard Session-Delegates (Create/Destroy/Find/Join)
		SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(
			this, &USteamMultiplayerSubsystem::HandleCreateSessionComplete);
		SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(
			this, &USteamMultiplayerSubsystem::HandleDestroySessionComplete);
		SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(
			this, &USteamMultiplayerSubsystem::HandleFindSessionsComplete);
		SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(
			this, &USteamMultiplayerSubsystem::HandleJoinSessionComplete);

		// Invite-Delegates mit Handles, damit wir sie bei Deinitialize aufräumen können
		InviteAcceptedHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
			FOnSessionUserInviteAcceptedDelegate::CreateUObject(
				this, &USteamMultiplayerSubsystem::HandleSessionUserInviteAccepted));

		InviteReceivedHandle = SessionInterface->AddOnSessionInviteReceivedDelegate_Handle(
			FOnSessionInviteReceivedDelegate::CreateUObject(
				this, &USteamMultiplayerSubsystem::HandleSessionInviteReceived));
	}
	else
	{
		UE_LOG(LogSteamMultiplayer, Warning, TEXT("SteamMultiplayerSubsystem: SessionInterface invalid."));
	}
}

void USteamMultiplayerSubsystem::Deinitialize()
{
	if (SessionInterface.IsValid())
	{
		if (InviteAcceptedHandle.IsValid())
		{
			SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(InviteAcceptedHandle);
		}
		if (InviteReceivedHandle.IsValid())
		{
			SessionInterface->ClearOnSessionInviteReceivedDelegate_Handle(InviteReceivedHandle);
		}
	}

	SessionInterface.Reset();
	ExternalUI.Reset();
	SessionSearch.Reset();
	LastSessionInfos.Reset();

	Super::Deinitialize();
}

bool USteamMultiplayerSubsystem::IsSteamOnline() const
{
	// Wenn wir irgendein OnlineSubsystem haben, sind wir "online".
	// Spezifisch Steam: CachedSubsystemName == "STEAM"
	return CachedSubsystemName != NAME_None;
}

bool USteamMultiplayerSubsystem::HasActiveSession() const
{
	if (!SessionInterface.IsValid())
	{
		return false;
	}

	const FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	return (ExistingSession != nullptr);
}

void USteamMultiplayerSubsystem::CreateSession(const FString& ServerName, int32 MaxPublicConnections)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogSteamMultiplayer, Warning, TEXT("CreateSession: SessionInterface invalid."));
		OnSessionCreated.Broadcast(false);
		return;
	}

	if (ServerName.IsEmpty())
	{
		UE_LOG(LogSteamMultiplayer, Warning, TEXT("CreateSession: ServerName is empty."));
		OnSessionCreated.Broadcast(false);
		return;
	}

	// Wenn bereits eine Session existiert → zuerst zerstören, danach neu erstellen
	if (HasActiveSession())
	{
		UE_LOG(LogSteamMultiplayer, Log, TEXT("CreateSession: Active session exists, destroying first..."));
		bPendingCreateAfterDestroy = true;
		PendingServerName = ServerName;
		PendingMaxPublicConnections = MaxPublicConnections;

		SessionInterface->DestroySession(NAME_GameSession);
		return;
	}

	const TSharedPtr<FOnlineSessionSettings> SessionSettings = BuildSessionSettings(MaxPublicConnections, ServerName);
	if (!SessionSettings.IsValid())
	{
		UE_LOG(LogSteamMultiplayer, Error, TEXT("CreateSession: Failed to build SessionSettings."));
		OnSessionCreated.Broadcast(false);
		return;
	}

	const TSharedPtr<const FUniqueNetId> UserId = GetLocalUserId();
	if (!UserId.IsValid())
	{
		UE_LOG(LogSteamMultiplayer, Warning, TEXT("CreateSession: LocalUserId invalid (not logged in?)."));
		OnSessionCreated.Broadcast(false);
		return;
	}

	UE_LOG(LogSteamMultiplayer, Log, TEXT("CreateSession: Creating session '%s' with %d connections."),
	       *ServerName, MaxPublicConnections);

	const bool bCreateStarted = SessionInterface->CreateSession(*UserId, NAME_GameSession, *SessionSettings);
	if (!bCreateStarted)
	{
		UE_LOG(LogSteamMultiplayer, Error, TEXT("CreateSession: CreateSession returned false immediately."));
		OnSessionCreated.Broadcast(false);
	}
}

void USteamMultiplayerSubsystem::FindSessions(int32 MaxResults, const FString& ServerNameFilter)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogSteamMultiplayer, Warning, TEXT("FindSessions: SessionInterface invalid."));
		OnSessionsFound.Broadcast(TArray<FSteamSessionInfo>(), false);
		return;
	}

	const TSharedPtr<const FUniqueNetId> UserId = GetLocalUserId();
	if (!UserId.IsValid())
	{
		UE_LOG(LogSteamMultiplayer, Warning, TEXT("FindSessions: LocalUserId invalid (not logged in?)."));
		OnSessionsFound.Broadcast(TArray<FSteamSessionInfo>(), false);
		return;
	}

	SessionSearch = MakeShared<FOnlineSessionSearch>();
	SessionSearch->MaxSearchResults = MaxResults;

	// LAN vs Online über Subsystem-Name (NULL = LAN)
	const bool bIsLanQuery = (CachedSubsystemName == NAME_None || CachedSubsystemName == TEXT("NULL"));
	SessionSearch->bIsLanQuery = bIsLanQuery;

	// Für Steam-Lobbies: SEARCH_LOBBIES = true setzen 
	SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

	CurrentServerNameFilter = ServerNameFilter;

	UE_LOG(LogSteamMultiplayer, Log, TEXT("FindSessions: Starting search (MaxResults=%d, Filter='%s')."),
	       MaxResults, *ServerNameFilter);

	const bool bSearchStarted = SessionInterface->FindSessions(*UserId, SessionSearch.ToSharedRef());
	if (!bSearchStarted)
	{
		UE_LOG(LogSteamMultiplayer, Error, TEXT("FindSessions: FindSessions returned false immediately."));
		OnSessionsFound.Broadcast(TArray<FSteamSessionInfo>(), false);
	}
}

void USteamMultiplayerSubsystem::JoinSessionBySearchResultIndex(int32 SearchResultIndex)
{
	if (!SessionInterface.IsValid() || !SessionSearch.IsValid())
	{
		UE_LOG(LogSteamMultiplayer, Warning,
		       TEXT("JoinSessionBySearchResultIndex: No SessionInterface or no valid search."));
		OnSessionJoined.Broadcast(false);
		return;
	}

	const TArray<FOnlineSessionSearchResult>& Results = SessionSearch->SearchResults;
	if (!Results.IsValidIndex(SearchResultIndex))
	{
		UE_LOG(LogSteamMultiplayer, Warning, TEXT("JoinSessionBySearchResultIndex: Index %d out of bounds."),
		       SearchResultIndex);
		OnSessionJoined.Broadcast(false);
		return;
	}

	FOnlineSessionSearchResult ChosenResult = Results[SearchResultIndex];
	if (!ChosenResult.IsValid())
	{
		UE_LOG(LogSteamMultiplayer, Warning,
		       TEXT("JoinSessionBySearchResultIndex: SearchResult at index %d is invalid."), SearchResultIndex);
		OnSessionJoined.Broadcast(false);
		return;
	}

	// Workaround für UE 5.5+: Presence/Lobbies müssen zum Host passen 
	ChosenResult.Session.SessionSettings.bUsesPresence = true;
	ChosenResult.Session.SessionSettings.bUseLobbiesIfAvailable = true;

	const TSharedPtr<const FUniqueNetId> UserId = GetLocalUserId();
	if (!UserId.IsValid())
	{
		UE_LOG(LogSteamMultiplayer, Warning, TEXT("JoinSessionBySearchResultIndex: LocalUserId invalid."));
		OnSessionJoined.Broadcast(false);
		return;
	}

	// Falls wir bereits in einer Session sind → erst zerstören, dann joinen
	if (HasActiveSession())
	{
		UE_LOG(LogSteamMultiplayer, Log,
		       TEXT("JoinSessionBySearchResultIndex: Active session exists, destroying first..."));

		bPendingJoinAfterDestroy = true;
		PendingJoinResult = ChosenResult;

		SessionInterface->DestroySession(NAME_GameSession);
		return;
	}

	const bool bJoinStarted = SessionInterface->JoinSession(*UserId, NAME_GameSession, ChosenResult);
	if (!bJoinStarted)
	{
		UE_LOG(LogSteamMultiplayer, Error,
		       TEXT("JoinSessionBySearchResultIndex: JoinSession returned false immediately."));
		OnSessionJoined.Broadcast(false);
	}
}

void USteamMultiplayerSubsystem::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogSteamMultiplayer, Warning, TEXT("DestroySession: SessionInterface invalid."));
		OnSessionDestroyed.Broadcast(false);
		return;
	}

	if (!HasActiveSession())
	{
		UE_LOG(LogSteamMultiplayer, Log, TEXT("DestroySession: No active session to destroy."));
		OnSessionDestroyed.Broadcast(true);
		return;
	}

	UE_LOG(LogSteamMultiplayer, Log, TEXT("DestroySession: Destroying NAME_GameSession..."));
	const bool bDestroyStarted = SessionInterface->DestroySession(NAME_GameSession);
	if (!bDestroyStarted)
	{
		UE_LOG(LogSteamMultiplayer, Error, TEXT("DestroySession: DestroySession returned false immediately."));
		OnSessionDestroyed.Broadcast(false);
	}
}

void USteamMultiplayerSubsystem::ShowInviteUI()
{
	if (!ExternalUI.IsValid())
	{
		UE_LOG(LogSteamMultiplayer, Warning, TEXT("ShowInviteUI: ExternalUI invalid (Steam overlay not available?)."));
		return;
	}

	// ControllerId 0 (erster lokaler Player), SessionName = NAME_GameSession
	ExternalUI->ShowInviteUI(0, NAME_GameSession);
}

TSharedPtr<const FUniqueNetId> USteamMultiplayerSubsystem::GetLocalUserId() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController();
	if (!LocalPlayer || !LocalPlayer->GetPreferredUniqueNetId().IsValid())
	{
		return nullptr;
	}

	return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}

void USteamMultiplayerSubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogSteamMultiplayer, Log, TEXT("HandleCreateSessionComplete: SessionName='%s', Success=%s"),
	       *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

	if (!bWasSuccessful)
	{
		OnSessionCreated.Broadcast(false);
		return;
	}

	OnSessionCreated.Broadcast(true);

	// Optional: Host direkt zur Lobby-Map traveln
	if (bTravelToLobbyOnCreate)
	{
		TravelToLobbyIfConfigured();
	}
}

void USteamMultiplayerSubsystem::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogSteamMultiplayer, Log, TEXT("HandleDestroySessionComplete: SessionName='%s', Success=%s"),
	       *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

	OnSessionDestroyed.Broadcast(bWasSuccessful);

	// Nach Destroy ggf. Create/Join fortsetzen
	if (bPendingCreateAfterDestroy && bWasSuccessful)
	{
		const FString ServerName = PendingServerName;
		const int32 MaxConnections = PendingMaxPublicConnections;

		bPendingCreateAfterDestroy = false;
		PendingServerName.Empty();
		PendingMaxPublicConnections = 0;

		UE_LOG(LogSteamMultiplayer, Log, TEXT("HandleDestroySessionComplete: Continuing pending CreateSession..."));
		CreateSession(ServerName, MaxConnections);
		return;
	}

	if (bPendingJoinAfterDestroy && bWasSuccessful && SessionInterface.IsValid())
	{
		bPendingJoinAfterDestroy = false;

		// Sicherheitsnetz für UE 5.5+ (Presence/Lobbies) 
		PendingJoinResult.Session.SessionSettings.bUsesPresence = true;
		PendingJoinResult.Session.SessionSettings.bUseLobbiesIfAvailable = true;

		const TSharedPtr<const FUniqueNetId> UserId = GetLocalUserId();
		if (!UserId.IsValid())
		{
			UE_LOG(LogSteamMultiplayer, Warning,
			       TEXT("HandleDestroySessionComplete: LocalUserId invalid for pending join."));
			OnSessionJoined.Broadcast(false);
			return;
		}

		UE_LOG(LogSteamMultiplayer, Log, TEXT("HandleDestroySessionComplete: Joining session after destroy..."));
		const bool bJoinStarted = SessionInterface->JoinSession(*UserId, NAME_GameSession, PendingJoinResult);
		if (!bJoinStarted)
		{
			UE_LOG(LogSteamMultiplayer, Error, TEXT("HandleDestroySessionComplete: JoinSession returned false."));
			OnSessionJoined.Broadcast(false);
		}
	}
}

void USteamMultiplayerSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
	UE_LOG(LogSteamMultiplayer, Log, TEXT("HandleFindSessionsComplete: Success=%s"),
	       bWasSuccessful ? TEXT("true") : TEXT("false"));

	LastSessionInfos.Reset();

	if (!bWasSuccessful || !SessionSearch.IsValid())
	{
		OnSessionsFound.Broadcast(LastSessionInfos, false);
		return;
	}

	// Workaround UE 5.5+: bUsesPresence/bUseLobbiesIfAvailable anpassen 
	for (FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
	{
		if (!Result.IsValid())
		{
			continue;
		}
		Result.Session.SessionSettings.bUsesPresence = true;
		Result.Session.SessionSettings.bUseLobbiesIfAvailable = true;
	}

	BuildLastSessionInfosFromSearchResults();
	OnSessionsFound.Broadcast(LastSessionInfos, true);
}

void USteamMultiplayerSubsystem::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	const bool bSuccess = (Result == EOnJoinSessionCompleteResult::Success);

	UE_LOG(LogSteamMultiplayer, Log, TEXT("HandleJoinSessionComplete: SessionName='%s', Result=%d (Success=%s)"),
	       *SessionName.ToString(), static_cast<int32>(Result), bSuccess ? TEXT("true") : TEXT("false"));

	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogSteamMultiplayer, Error, TEXT("HandleJoinSessionComplete: SessionInterface invalid."));
		OnSessionJoined.Broadcast(false);
		return;
	}

	if (!bSuccess)
	{
		OnSessionJoined.Broadcast(false);
		return;
	}

	// ConnectString (IP/Steam-URL) auflösen und ClientTravel durchführen
	FString ConnectString;
	bool bGotConnectString = SessionInterface->GetResolvedConnectString(SessionName, ConnectString, NAME_GamePort);
	if (!bGotConnectString)
	{
		// Fallback: ohne speziellen Portnamen
		bGotConnectString = SessionInterface->GetResolvedConnectString(SessionName, ConnectString);
	}

	if (!bGotConnectString || ConnectString.IsEmpty())
	{
		UE_LOG(LogSteamMultiplayer, Error, TEXT("HandleJoinSessionComplete: GetResolvedConnectString failed."));
		OnSessionJoined.Broadcast(false);
		return;
	}

	UE_LOG(LogSteamMultiplayer, Log, TEXT("HandleJoinSessionComplete: Traveling client to '%s'"), *ConnectString);

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogSteamMultiplayer, Error, TEXT("HandleJoinSessionComplete: GameInstance is null."));
		OnSessionJoined.Broadcast(false);
		return;
	}

	APlayerController* PC = GI->GetFirstLocalPlayerController();
	if (!PC)
	{
		UE_LOG(LogSteamMultiplayer, Error, TEXT("HandleJoinSessionComplete: No local PlayerController."));
		OnSessionJoined.Broadcast(false);
		return;
	}

	PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
	OnSessionJoined.Broadcast(true);
}

void USteamMultiplayerSubsystem::HandleSessionInviteReceived(
	const FUniqueNetId& UserId,
	const FUniqueNetId& FromId,
	const FString& AppId,
	const FOnlineSessionSearchResult& InviteResult)
{
	UE_LOG(LogSteamMultiplayer, Log, TEXT("HandleSessionInviteReceived: FromId=%s"),
	       *FromId.ToString());
	// Hier könntest du bei Bedarf ein eigenes UI triggern (Popup etc.).
}

void USteamMultiplayerSubsystem::HandleSessionUserInviteAccepted(
	bool bWasSuccessful,
	int32 ControllerId,
	TSharedPtr<const FUniqueNetId> UserId,
	const FOnlineSessionSearchResult& InviteResult)
{
	UE_LOG(LogSteamMultiplayer, Log, TEXT("HandleSessionUserInviteAccepted: Success=%s"),
	       bWasSuccessful ? TEXT("true") : TEXT("false"));

	if (!bWasSuccessful || !InviteResult.IsValid() || !SessionInterface.IsValid())
	{
		OnSessionJoined.Broadcast(false);
		return;
	}

	// Workaround: Presence/Lobbies für InviteResult korrigieren 
	FOnlineSessionSearchResult FixedInvite = InviteResult;
	FixedInvite.Session.SessionSettings.bUsesPresence = true;
	FixedInvite.Session.SessionSettings.bUseLobbiesIfAvailable = true;

	// Fallback, falls UserId leer ist:
	if (!UserId.IsValid())
	{
		UserId = GetLocalUserId();
		if (!UserId.IsValid())
		{
			UE_LOG(LogSteamMultiplayer, Error, TEXT("InviteAccepted: LocalUserId invalid."));
			OnSessionJoined.Broadcast(false);
			return;
		}
	}

	// Wenn bereits in einer Session → erst zerstören, dann joinen
	if (HasActiveSession())
	{
		UE_LOG(LogSteamMultiplayer, Log,
		       TEXT("InviteAccepted: Active session exists, destroying then joining invite."));

		bPendingJoinAfterDestroy = true;
		PendingJoinResult = FixedInvite;

		SessionInterface->DestroySession(NAME_GameSession);
		return;
	}

	const bool bJoinStarted = SessionInterface->JoinSession(*UserId, NAME_GameSession, FixedInvite);
	if (!bJoinStarted)
	{
		UE_LOG(LogSteamMultiplayer, Error, TEXT("InviteAccepted: JoinSession returned false."));
		OnSessionJoined.Broadcast(false);
	}
}

void USteamMultiplayerSubsystem::BuildLastSessionInfosFromSearchResults()
{
	LastSessionInfos.Reset();

	if (!SessionSearch.IsValid())
	{
		return;
	}

	const TArray<FOnlineSessionSearchResult>& Results = SessionSearch->SearchResults;

	for (int32 Index = 0; Index < Results.Num(); ++Index)
	{
		const FOnlineSessionSearchResult& Result = Results[Index];
		if (!Result.IsValid())
		{
			continue;
		}

		// Optionaler Name-Filter
		FString ServerName;
		Result.Session.SessionSettings.Get(SETTING_SERVER_NAME, ServerName);

		if (!CurrentServerNameFilter.IsEmpty())
		{
			if (!ServerName.Equals(CurrentServerNameFilter, ESearchCase::IgnoreCase))
			{
				continue;
			}
		}

		FSteamSessionInfo Info;
		Info.ServerName = ServerName;
		Info.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
		Info.PingInMs = Result.PingInMs;
		Info.SearchResultIndex = Index;

		const int32 OpenPublicConnections = Result.Session.NumOpenPublicConnections;
		Info.CurrentPlayers = Info.MaxPlayers - OpenPublicConnections;

		// HostUserName (optional, falls vorhanden)
		if (Result.Session.OwningUserName.Len() > 0)
		{
			Info.HostUserName = Result.Session.OwningUserName;
		}

		LastSessionInfos.Add(Info);
	}
}

TSharedPtr<FOnlineSessionSettings> USteamMultiplayerSubsystem::BuildSessionSettings(
	int32 MaxPublicConnections,
	const FString& ServerName) const
{
	TSharedPtr<FOnlineSessionSettings> Settings = MakeShared<FOnlineSessionSettings>();

	// LAN vs Online
	const bool bIsLan = (CachedSubsystemName == NAME_None || CachedSubsystemName == TEXT("NULL"));
	Settings->bIsLANMatch = bIsLan;

	Settings->bIsDedicated = false;
	Settings->NumPublicConnections = MaxPublicConnections;
	Settings->bAllowJoinInProgress = true;
	Settings->bShouldAdvertise = true;
	Settings->bAllowInvites = true;

	// Für Steam-Lobbies: Presence & Lobbies aktivieren 
	Settings->bUsesPresence = true;
	Settings->bUseLobbiesIfAvailable = true;
	Settings->bAllowJoinViaPresence = true;
	Settings->bAllowJoinViaPresenceFriendsOnly = false;

	// Anzeigename
	Settings->Set(SETTING_SERVER_NAME, ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	return Settings;
}

void USteamMultiplayerSubsystem::TravelToLobbyIfConfigured()
{
	if (LobbyMapPath.IsEmpty())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogSteamMultiplayer, Warning, TEXT("TravelToLobbyIfConfigured: World is null."));
		return;
	}

	// Pfad ohne ?listen, z.B. "/Game/Maps/Lobby"
	const FString MapPath = LobbyMapPath;
	const FString AssetName = FPaths::GetCleanFilename(MapPath); // "Lobby"
	const FString MapObjectRef = MapPath + TEXT(".") + AssetName; // "/Game/Maps/Lobby.Lobby"
	const FString TravelURL = MapPath + TEXT("?listen");

	// Workaround (optional): Default-Map vor Travel umstellen, um gewisse 5.6/5.7 Bugs zu umgehen. 
	UGameMapsSettings::SetGameDefaultMap(MapObjectRef);

	UE_LOG(LogSteamMultiplayer, Log, TEXT("TravelToLobbyIfConfigured: ServerTravel to '%s'"), *TravelURL);
	World->ServerTravel(TravelURL);
}
