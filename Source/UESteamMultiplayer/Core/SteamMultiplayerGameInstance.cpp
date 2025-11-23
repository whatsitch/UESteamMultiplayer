// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/SteamMultiplayerGameInstance.h"

void USteamMultiplayerGameInstance::Init()
{
	Super::Init();

	SteamSubsystem = GetSubsystem<USteamMultiplayerSubsystem>();

	if (SteamSubsystem)
	{
		SteamSubsystem->LobbyMapPath = TEXT("/Game/Maps/MainMenu");
		SteamSubsystem->bTravelToLobbyOnCreate = true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GameInstance: SteamMultiplayerSubsystem not found"));
	}
}

void USteamMultiplayerGameInstance::Shutdown()
{
	SteamSubsystem = nullptr;
	Super::Shutdown();
}
