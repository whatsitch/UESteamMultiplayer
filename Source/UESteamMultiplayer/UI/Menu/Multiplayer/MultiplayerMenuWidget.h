// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Lobby/LobbyGameState.h"
#include "UI/Menu/Lobby/LobbyPlayerEntryWidget.h"
#include "MultiplayerMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class UESTEAMMULTIPLAYER_API UMultiplayerMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta=(BindWidget))
	UWidgetSwitcher* LeftViewSwitcher;
	
	/*----- Default View (0) -----*/

	UPROPERTY(meta=(BindWidget))
	UButton* CreateGameButton;

	UPROPERTY(meta=(BindWidget))
	UButton* JoinGameButton;
	
	/*----- View Create Game (1) -----*/
	UPROPERTY(meta=(BindWidget))
	UButton* StartGameButton;
	

	/*----- global elements -----*/
	UPROPERTY(meta=(BindWidget))
	UButton* BackButton;

	UPROPERTY(meta = (BindWidget))
	UPanelWidget* PlayerListPanel; // z.B. VerticalBox

	UPROPERTY(meta = (BindWidget))
	UButton* InviteFriendButton;

	UPROPERTY(EditDefaultsOnly, Category="Lobby")
	TSubclassOf<ULobbyPlayerEntryWidget> PlayerEntryWidgetClass;

	UFUNCTION()
	void HandleCreateGameClicked();

	UFUNCTION()
	void HandleJoinGameClicked();

	UFUNCTION()
	void HandleBackClicked();

	UFUNCTION()
	void HandleInviteFriendClicked();
	
	UFUNCTION()
	void HandleStartGameClicked();

private:
	
	enum class EMultiplayerMenuView : uint8
	{
		Root = 0,
		CreateLobby = 1,
		ServerBrowser = 2
	};

	void SwitchToView(EMultiplayerMenuView NewView);
	
	/** -----Current LobbyGameState reference (World dependent)----- */
	TWeakObjectPtr<ALobbyGameState> CachedLobbyGameState;

	/** -----Called from the GameState-Delegate----- */
	void HandleLobbyPlayersChanged();

	/**----- Re-Render the lobby list -----*/
	void RefreshPlayerList();
};
