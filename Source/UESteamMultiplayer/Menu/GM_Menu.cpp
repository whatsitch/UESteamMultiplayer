// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu/GM_Menu.h"
#include "Menu/PC_Menu.h"
#include "GameFramework/PlayerState.h"
#include "Lobby/LobbyGameState.h"

AGM_Menu::AGM_Menu()
{
	PlayerControllerClass = APC_Menu::StaticClass();
	GameStateClass = ALobbyGameState::StaticClass();
}

void AGM_Menu::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ALobbyGameState* GS = GetGameState<ALobbyGameState>();
	if (GS && GS->HostPlayerState == nullptr && NewPlayer && NewPlayer->PlayerState)
	{
		GS->HostPlayerState = NewPlayer->PlayerState;

		UE_LOG(LogTemp, Log, TEXT("GM_Menu: HostPlayerState set to %s"),
			   *NewPlayer->PlayerState->GetPlayerName());
	}
}
