// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "MainMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class UESTEAMMULTIPLAYER_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta=(BindWidget))
	UButton* SingleplayerButton;

	UPROPERTY(meta=(BindWidget))
	UButton* MultiplayerButton;

	UPROPERTY(meta=(BindWidget))
	UButton* QuitButton;

private:
	UFUNCTION()
	void HandleSingleplayerClicked();

	UFUNCTION()
	void HandleMultiplayerClicked();

	UFUNCTION()
	void HandleQuitClicked();
};
