#pragma once

#include "CoreMinimal.h"
#include "SteamMultiplayerSubsystem.h"
#include "Engine/GameInstance.h"
#include "SteamMultiplayerGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class UESTEAMMULTIPLAYER_API USteamMultiplayerGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	UFUNCTION(BlueprintPure, Category="Multiplayer")
	USteamMultiplayerSubsystem* GetSteamMultiplayerSubsystem() const { return SteamSubsystem; }

private:
	UPROPERTY()
	USteamMultiplayerSubsystem* SteamSubsystem = nullptr;
};
