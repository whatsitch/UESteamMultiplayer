// Fill out your copyright notice in the Description page of Project Settings.


#include "SteamMultiplayerSubsystem.h"

#include "OnlineSubsystemUtils.h"

void USteamMultiplayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	OnlineSubsystem = Online::GetSubsystem(GetWorld());

	if (OnlineSubsystem)
	{
		const FName SubsystemName = OnlineSubsystem->GetSubsystemName();
		UE_LOG(LogTemp, Log, TEXT("SteamMultiplayerSubsystem: OnlineSubsystem = %s"), *SubsystemName.ToString());

		SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (!SessionInterface.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("SteamMultiplayerSubsystem: SessionInterface invalid"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SteamMultiplayerSubsystem: Kein OnlineSubsystem gefunden (erwartet: Steam oder Null)."));
	}
}

void USteamMultiplayerSubsystem::Deinitialize()
{
	SessionInterface.Reset();
	OnlineSubsystem = nullptr;

	Super::Deinitialize();
}

bool USteamMultiplayerSubsystem::IsUsingSteam() const
{
	if (!OnlineSubsystem)
	{
		return false;
	}
	return OnlineSubsystem->GetSubsystemName() == TEXT("STEAM");
}