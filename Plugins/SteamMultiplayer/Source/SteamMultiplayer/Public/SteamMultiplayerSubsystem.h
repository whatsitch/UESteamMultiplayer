// SteamMultiplayerSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "OnlineSessionSettings.h"
#include "SteamMultiplayerSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSteamMultiplayer, Log, All);

/**
 * Vereinfachte Blueprint-freundliche Session-Info für UI.
 */
USTRUCT(BlueprintType)
struct STEAMMULTIPLAYER_API FSteamSessionInfo
{
	GENERATED_BODY()

	/** Anzeigename des Servers (SERVER_NAME in SessionSettings) */
	UPROPERTY(BlueprintReadOnly)
	FString ServerName;

	/** Host-Name (optional, je nach Plattform/Subsystem) */
	UPROPERTY(BlueprintReadOnly)
	FString HostUserName;

	/** Aktuelle Spielerzahl (öffentlich) */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPlayers = 0;

	/** Maximale Spielerzahl (NumPublicConnections) */
	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayers = 0;

	/** Ping in Millisekunden */
	UPROPERTY(BlueprintReadOnly)
	int32 PingInMs = 0;

	/** Index in der aktuellen SearchResults-Liste (für Join) */
	UPROPERTY(BlueprintReadOnly)
	int32 SearchResultIndex = INDEX_NONE;
};

/** Session wurde erstellt (true = Erfolg) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSteamSessionCreateDelegate, bool, bWasSuccessful);

/** Session wurde zerstört (true = Erfolg) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSteamSessionDestroyDelegate, bool, bWasSuccessful);

/** Sessionsuche abgeschlossen: Ergebnisse + Erfolg */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FSteamSessionsFindDelegate,
	const TArray<FSteamSessionInfo>&, Results,
	bool, bWasSuccessful);

/** Join-Versuch abgeschlossen (true = Erfolg) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSteamSessionJoinDelegate, bool, bWasSuccessful);


/**
 * SteamMultiplayerSubsystem
 * - Kapselt OnlineSubsystem (Steam/Null)
 * - Erstellt/Findet/Joined Sessions
 * - Handhabt Steam-Invites
 */
UCLASS()
class STEAMMULTIPLAYER_API USteamMultiplayerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USteamMultiplayerSubsystem();

	//~UGameInstanceSubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End UGameInstanceSubsystem

	/** Ist Steam (oder generelles OSS) aktiv? */
	UFUNCTION(BlueprintPure, Category="Steam|Sessions")
	bool IsSteamOnline() const;

	/** Hat der lokale Spieler aktuell eine aktive Session (NAME_GameSession)? */
	UFUNCTION(BlueprintPure, Category="Steam|Sessions")
	bool HasActiveSession() const;

	/**
	 * Session/Lobby erstellen.
	 * @param ServerName  Anzeigename des Servers (wird als SERVER_NAME in SessionSettings gesetzt)
	 * @param MaxPublicConnections Anzahl öffentlicher Slots (inkl. Host)
	 */
	UFUNCTION(BlueprintCallable, Category="Steam|Sessions")
	void CreateSession(const FString& ServerName, int32 MaxPublicConnections = 4);

	/**
	 * Sessions suchen.
	 * @param MaxResults             Maximale Anzahl Suchresultate
	 * @param ServerNameFilter       Optionaler Name-Filter (SERVER_NAME, case-insensitive).
	 *                               Leer = alle Sessions zurückgeben.
	 */
	UFUNCTION(BlueprintCallable, Category="Steam|Sessions")
	void FindSessions(int32 MaxResults = 100, const FString& ServerNameFilter = TEXT(""));

	/**
	 * Session joinen anhand eines Suchresultat-Index (aus FSteamSessionInfo.SearchResultIndex).
	 */
	UFUNCTION(BlueprintCallable, Category="Steam|Sessions")
	void JoinSessionBySearchResultIndex(int32 SearchResultIndex);

	/**
	 * Aktuelle Session zerstören (falls vorhanden).
	 */
	UFUNCTION(BlueprintCallable, Category="Steam|Sessions")
	void DestroySession();

	/**
	 * Steam-Invite-Overlay anzeigen (Freunde einladen).
	 */
	UFUNCTION(BlueprintCallable, Category="Steam|Invites")
	void ShowInviteUI();

	/**
	 * Letzte gefundene Sessions (vereinfacht) zurückgeben.
	 * Praktisch, falls du die Ergebnisse z.B. manuell in einem UI verarbeiten willst.
	 */
	UFUNCTION(BlueprintPure, Category="Steam|Sessions")
	const TArray<FSteamSessionInfo>& GetLastSessionInfos() const { return LastSessionInfos; }

	/* */
	UFUNCTION(BlueprintCallable, Category="Steam|Sessions")
	void TravelToLobby();

	/** Map, zu der der Host nach erfolgreichem Create traveln soll (z.B. "/Game/Maps/Lobby") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Steam|Sessions")
	FString LobbyMapPath;

	/** Ob nach erfolgreichem Create automatisch zu LobbyMapPath getravelt werden soll */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Steam|Sessions")
	bool bTravelToLobbyOnCreate = true;

	/** BP-Event: Session Erstellung */
	UPROPERTY(BlueprintAssignable, Category="Steam|Events")
	FSteamSessionCreateDelegate OnSessionCreated;

	/** BP-Event: Session Zerstörung */
	UPROPERTY(BlueprintAssignable, Category="Steam|Events")
	FSteamSessionDestroyDelegate OnSessionDestroyed;

	/** BP-Event: Sessions gefunden */
	UPROPERTY(BlueprintAssignable, Category="Steam|Events")
	FSteamSessionsFindDelegate OnSessionsFound;

	/** BP-Event: Join Ergebnis */
	UPROPERTY(BlueprintAssignable, Category="Steam|Events")
	FSteamSessionJoinDelegate OnSessionJoined;

private:
	/** Hilfsfunktion: lokale UserId holen (Steam-/Null-Subsystem) */
	TSharedPtr<const FUniqueNetId> GetLocalUserId() const;

	/** SessionInterface des aktuellen OnlineSubsystems (Steam/Null) */
	IOnlineSessionPtr SessionInterface;

	/** External UI (für Steam-Invite-Overlay) */
	IOnlineExternalUIPtr ExternalUI;

	/** Letzte Suchanfrage (vollständig, inkl. SearchResults) */
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	/** Vereinfachte Kopie der letzten Suchresultate für BP/UI */
	TArray<FSteamSessionInfo> LastSessionInfos;

	/** Optionaler ServerName-Filter bei der aktuellen Suche */
	FString CurrentServerNameFilter;

	/** Pending-Flags für "Destroy dann Create" */
	bool bPendingCreateAfterDestroy = false;
	FString PendingServerName;
	int32 PendingMaxPublicConnections = 0;

	/** Pending-Flags für "Destroy dann Join" */
	bool bPendingJoinAfterDestroy = false;
	FOnlineSessionSearchResult PendingJoinResult;

	/** Invite-Delegate Handles, damit wir sauber aufräumen können */
	FDelegateHandle InviteAcceptedHandle;
	FDelegateHandle InviteReceivedHandle;

	/** OnlineSubsystem-Name (z.B. "STEAM" oder "NULL") – nur als Cache */
	FName CachedSubsystemName = NAME_None;

	/** SessionSettings-Key für unseren Anzeigenamen */
	static const FName SETTING_SERVER_NAME;

	/** Interne Callbacks für Session-Delegates */
	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	/** Invite-Callbacks */
	void HandleSessionInviteReceived(
		const FUniqueNetId& UserId,
		const FUniqueNetId& FromId,
		const FString& AppId,
		const FOnlineSessionSearchResult& InviteResult);

	void HandleSessionUserInviteAccepted(
		bool bWasSuccessful,
		int32 ControllerId,
		TSharedPtr<const FUniqueNetId> UserId,
		const FOnlineSessionSearchResult& InviteResult);

	/** Suche in BP-freundliche Structs umwandeln */
	void BuildLastSessionInfosFromSearchResults();

	/** FOnlineSessionSettings für CreateSession vorbereiten */
	TSharedPtr<FOnlineSessionSettings>
	BuildSessionSettings(int32 MaxPublicConnections, const FString& ServerName) const;

	/** Host nach erfolgreichem Create zur Lobby-Map traveln */
	void TravelToLobbyIfConfigured();
};
