// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
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

	UPROPERTY(meta=(BindWidget))
	UButton* BackButton;

	UFUNCTION()
	void HandleBackClicked();
	
};
