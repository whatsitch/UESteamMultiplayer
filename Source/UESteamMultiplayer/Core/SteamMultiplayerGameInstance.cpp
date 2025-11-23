// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/SteamMultiplayerGameInstance.h"

void USteamMultiplayerGameInstance::Init()
{
	Super::Init();

	SteamSubsystem = GetSubsystem<USteamMultiplayerSubsystem>();

	if (SteamSubsystem)
	{
		const bool bIsSteam = SteamSubsystem->IsUsingSteam();
		UE_LOG(LogTemp, Log, TEXT("GameInstance: SteamMultiplayerSubsystem gefunden. IsUsingSteam = %s"),
			   bIsSteam ? TEXT("true") : TEXT("false"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GameInstance: SteamMultiplayerSubsystem NICHT gefunden."));
	}
}

void USteamMultiplayerGameInstance::Shutdown()
{
	SteamSubsystem = nullptr;
	Super::Shutdown();
}