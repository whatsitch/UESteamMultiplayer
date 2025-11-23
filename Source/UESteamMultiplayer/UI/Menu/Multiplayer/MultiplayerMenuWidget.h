// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
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


	/*----- Buttons -----*/

	UPROPERTY(meta=(BindWidget))
	UButton* CreateGameButton;

	UPROPERTY(meta=(BindWidget))
	UButton* JoinGameButton;

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

private:
	/** -----Current LobbyGameState reference (World dependent)----- */
	TWeakObjectPtr<ALobbyGameState> CachedLobbyGameState;

	/** -----Called from the GameState-Delegate----- */
	void HandleLobbyPlayersChanged();

	/**----- Re-Render the lobby list -----*/
	void RefreshPlayerList();
};
