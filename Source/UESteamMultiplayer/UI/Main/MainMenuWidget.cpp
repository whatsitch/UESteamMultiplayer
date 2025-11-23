// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Main/MainMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("UMainMenuWidget::NativeConstruct"));
	}

	if (SingleplayerButton)
	{
		SingleplayerButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleSingleplayerClicked);
	}
	if (MultiplayerButton)
	{
		MultiplayerButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleMultiplayerClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleQuitClicked);
	}
}

void UMainMenuWidget::HandleSingleplayerClicked()
{
	UE_LOG(LogTemp, Log, TEXT("MainMenu: Singleplayer geklickt (TODO: später)."));
}

void UMainMenuWidget::HandleMultiplayerClicked()
{
	UE_LOG(LogTemp, Log, TEXT("MainMenu: Multiplayer geklickt (TODO: Multiplayer-Menü öffnen)."));
	// Hier später: UESM_GameInstance → Multiplayer-Menü öffnen
}

void UMainMenuWidget::HandleQuitClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
	}
}
