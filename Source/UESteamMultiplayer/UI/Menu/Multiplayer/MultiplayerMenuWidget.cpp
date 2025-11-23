#include "UI/Menu/Multiplayer/MultiplayerMenuWidget.h"
#include "SteamMultiplayerSubsystem.h"
#include "Lobby/LobbyGameState.h"
#include "Menu/PC_Menu.h"


void UMultiplayerMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CreateGameButton)
	{
		CreateGameButton->OnClicked.AddDynamic(this, &UMultiplayerMenuWidget::HandleCreateGameClicked);
	}
	if (JoinGameButton)
	{
		JoinGameButton->OnClicked.AddDynamic(this, &UMultiplayerMenuWidget::HandleJoinGameClicked);
	}
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UMultiplayerMenuWidget::HandleBackClicked);
	}
	if (InviteFriendButton)
	{
		InviteFriendButton->OnClicked.AddDynamic(this, &UMultiplayerMenuWidget::HandleInviteFriendClicked);
	}

	// 🔹 GameState holen & Event abonnieren
	if (UWorld* World = GetWorld())
	{
		if (ALobbyGameState* GameState = World->GetGameState<ALobbyGameState>())
		{
			CachedLobbyGameState = GameState;
			GameState->OnLobbyPlayersChanged.AddUObject(this, &UMultiplayerMenuWidget::HandleLobbyPlayersChanged);
		}
	}

	// 🔹 Beim ersten Öffnen einmal die Liste aufbauen
	RefreshPlayerList();
}

void UMultiplayerMenuWidget::NativeDestruct()
{
	// Delegate wieder abmelden, um Dangling-Callbacks zu vermeiden
	if (CachedLobbyGameState.IsValid())
	{
		CachedLobbyGameState->OnLobbyPlayersChanged.RemoveAll(this);
		CachedLobbyGameState = nullptr;
	}

	Super::NativeDestruct();
}


void UMultiplayerMenuWidget::HandleCreateGameClicked()
{
	UE_LOG(LogTemp, Log, TEXT("MultiplayerMenu: Create Multiplayer Game clicked"));
}

void UMultiplayerMenuWidget::HandleJoinGameClicked()
{
	UE_LOG(LogTemp, Log, TEXT("MultiplayerMenu: Join Multiplayer Game clicked (TODO: FindSessions)"));
	UE_LOG(LogTemp, Log,
	       TEXT("MultiplayerMenu: Join Multiplayer Game clicked (TODO: Server-Browser via FindSessions)"));

	// TODO:
	// - Subsystem->FindSessions(...)
	// - Im OnSessionsFound-Delegate Ergebnisliste anzeigen
	// - Aus der Liste einen Eintrag auswählen und JoinSessionBySearchResultIndex(...) aufrufen
}

void UMultiplayerMenuWidget::HandleBackClicked()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		if (APC_Menu* MenuPC = Cast<APC_Menu>(PlayerController))
		{
			MenuPC->ShowMainMenu();
		}
	}
}

void UMultiplayerMenuWidget::HandleInviteFriendClicked()
{
	UE_LOG(LogTemp, Log, TEXT("MultiplayerMenu: InviteFriend clicked"));

	if (UGameInstance* GI = GetGameInstance())
	{
		if (USteamMultiplayerSubsystem* Subsystem = GI->GetSubsystem<USteamMultiplayerSubsystem>())
		{
			Subsystem->ShowInviteUI();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("MultiplayerMenu: SteamMultiplayerSubsystem not found"));
		}
	}
}

void UMultiplayerMenuWidget::HandleLobbyPlayersChanged()
{
	// Einfach Liste neu aufbauen, wenn sich was ändert
	RefreshPlayerList();
}

void UMultiplayerMenuWidget::RefreshPlayerList()
{
	if (!PlayerListPanel || !PlayerEntryWidgetClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ALobbyGameState* GS = CachedLobbyGameState.IsValid()
		                      ? CachedLobbyGameState.Get()
		                      : World->GetGameState<ALobbyGameState>();

	if (!GS)
	{
		return;
	}

	const TArray<APlayerState*>& Players = GS->PlayerArray;
	APlayerState* Host = GS->HostPlayerState;

	PlayerListPanel->ClearChildren();

	for (APlayerState* PS : Players)
	{
		if (!PS) continue;

		ULobbyPlayerEntryWidget* Entry =
			CreateWidget<ULobbyPlayerEntryWidget>(GetOwningPlayer(), PlayerEntryWidgetClass);

		if (!Entry) continue;

		const bool bIsHost = (PS == Host);
		Entry->InitFromPlayerState(PS, bIsHost);

		PlayerListPanel->AddChild(Entry);
	}
}
