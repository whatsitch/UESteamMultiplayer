// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GM_Menu.generated.h"

/**
 * 
 */
UCLASS()
class UESTEAMMULTIPLAYER_API AGM_Menu : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AGM_Menu();
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
};
