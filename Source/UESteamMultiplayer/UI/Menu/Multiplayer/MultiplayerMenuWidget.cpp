#include "UI/Menu/Multiplayer/MultiplayerMenuWidget.h"
#include "SteamMultiplayerSubsystem.h"
#include "Lobby/LobbyGameState.h"
#include "Menu/PC_Menu.h"


/**
 * Called when the widget is constructed.
 * Binds button click events, sets up the initial view, and subscribes to lobby player changes.
 */
void UMultiplayerMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind main menu buttons to their handler functions
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

	// Set default view to the root Create/Join view
	if (LeftViewSwitcher)
	{
		LeftViewSwitcher->SetActiveWidgetIndex(0);
	}


	// Get the LobbyGameState and subscribe to player changes to keep the UI updated
	if (UWorld* World = GetWorld())
	{
		if (ALobbyGameState* GameState = World->GetGameState<ALobbyGameState>())
		{
			CachedLobbyGameState = GameState;
			GameState->OnLobbyPlayersChanged.AddUObject(this, &UMultiplayerMenuWidget::HandleLobbyPlayersChanged);
		}
	}

	// Build initial player list if a lobby already exists
	RefreshPlayerList();
}

/**
 * Called when the widget is destroyed.
 * Unregisters delegates and clears cached references to avoid dangling callbacks.
 */
void UMultiplayerMenuWidget::NativeDestruct()
{
	// Unbind delegate to avoid dangling callbacks when the widget is destroyed
	if (CachedLobbyGameState.IsValid())
	{
		CachedLobbyGameState->OnLobbyPlayersChanged.RemoveAll(this);
		CachedLobbyGameState = nullptr;
	}

	Super::NativeDestruct();
}


/**
 * Handles the Create Game button click.
 * Switches the UI to the Create Lobby view.
 */
void UMultiplayerMenuWidget::HandleCreateGameClicked()
{
	UE_LOG(LogTemp, Log, TEXT("MultiplayerMenu: Create Game clicked -> switch to CreateLobby view"));
	SwitchToView(EMultiplayerMenuView::CreateLobby);
}

/**
 * Handles the Join Game button click.
 * Switches the UI to the Server Browser view and is the entry point for starting a session search.
 */
void UMultiplayerMenuWidget::HandleJoinGameClicked()
{
	UE_LOG(LogTemp, Log, TEXT("MultiplayerMenu: Join Game clicked -> switch to ServerBrowser view (TODO)"));
	UE_LOG(LogTemp, Log,
	       TEXT("MultiplayerMenu: Join Multiplayer Game clicked (TODO: Server-Browser via FindSessions)"));
	SwitchToView(EMultiplayerMenuView::ServerBrowser);

	// TODO:
	// - Subsystem->FindSessions(...)
	// - Im OnSessionsFound-Delegate Ergebnisliste anzeigen
	// - Aus der Liste einen Eintrag auswählen und JoinSessionBySearchResultIndex(...) aufrufen
}

/**
 * Handles the Back button click.
 * Navigates back to the previous view or to the main menu if at the root view.
 */
void UMultiplayerMenuWidget::HandleBackClicked()
{
	// If the switcher does not exist, behave like "back to main menu"
	if (!LeftViewSwitcher)
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (APC_Menu* MenuPC = Cast<APC_Menu>(PC))
			{
				MenuPC->ShowMainMenu();
			}
		}
		return;
	}

	const int32 CurrentIndex = LeftViewSwitcher->GetActiveWidgetIndex();

	if (CurrentIndex == static_cast<int32>(EMultiplayerMenuView::Root))
	{
		// We are in the root view -> go all the way back to the main menu
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (APC_Menu* MenuPC = Cast<APC_Menu>(PC))
			{
				MenuPC->ShowMainMenu();
			}
		}
	}
	else
	{
		// We are in a sub-view (CreateLobby / ServerBrowser) -> go back to the root view only
		SwitchToView(EMultiplayerMenuView::Root);
	}
}

/**
 * Handles the Invite Friend button click.
 * Opens the platform invite UI via the Steam multiplayer subsystem.
 */
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

/**
 * Switches the left view switcher to the given menu view.
 * Maps the logical enum to the underlying widget index.
 */
void UMultiplayerMenuWidget::SwitchToView(EMultiplayerMenuView NewView)
{
	if (!LeftViewSwitcher)
	{
		return;
	}

	int32 TargetIndex = 0;
	switch (NewView)
	{
	case EMultiplayerMenuView::Root:
		TargetIndex = 0;
		break;
	case EMultiplayerMenuView::CreateLobby:
		TargetIndex = 1;
		break;
	case EMultiplayerMenuView::ServerBrowser:
		TargetIndex = 2;
		break;
	}

	LeftViewSwitcher->SetActiveWidgetIndex(TargetIndex);
}

/**
 * Callback for lobby player changes.
 * Rebuilds the player list whenever the lobby composition changes.
 */
void UMultiplayerMenuWidget::HandleLobbyPlayersChanged()
{
	RefreshPlayerList();
}

/**
 * Rebuilds the UI list of lobby players based on the current LobbyGameState.
 * Creates one entry widget per player and marks the current host.
 */
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

	// Use cached LobbyGameState if available, otherwise try to get it from the world
	const ALobbyGameState* GameState = CachedLobbyGameState.IsValid()
		                                   ? CachedLobbyGameState.Get()
		                                   : World->GetGameState<ALobbyGameState>();

	if (!GameState)
	{
		return;
	}

	const TArray<APlayerState*>& Players = GameState->PlayerArray;
	APlayerState* Host = GameState->HostPlayerState;

	// Clear old entries before rebuilding
	PlayerListPanel->ClearChildren();

	for (APlayerState* PlayerState : Players)
	{
		if (!PlayerState) continue;

		// Create a new UI entry for each player in the lobby
		ULobbyPlayerEntryWidget* Entry =
			CreateWidget<ULobbyPlayerEntryWidget>(GetOwningPlayer(), PlayerEntryWidgetClass);

		if (!Entry) continue;

		const bool bIsHost = (PlayerState == Host);
		Entry->InitFromPlayerState(PlayerState, bIsHost);

		// Add the entry to the vertical list panel
		PlayerListPanel->AddChild(Entry);
	}
}
