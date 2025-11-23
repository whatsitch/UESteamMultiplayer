// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SteamMultiplayerSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class STEAMMULTIPLAYER_API USteamMultiplayerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	
	/** Nur zum Testen: gibt zurück, ob wirklich Steam aktiv ist */
	UFUNCTION(BlueprintCallable, Category="SteamMultiplayer")
	bool IsUsingSteam() const;

	/** Zugriff auf Session-Interface (für spätere Schritte) */
	IOnlineSessionPtr GetSessionInterface() const { return SessionInterface; }
	
	
private:
	IOnlineSubsystem* OnlineSubsystem = nullptr;
	IOnlineSessionPtr SessionInterface;
};
