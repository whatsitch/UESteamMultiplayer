// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Menu/Multiplayer/MultiplayerMenuWidget.h"

#include "Menu/PC_Menu.h"


void UMultiplayerMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UMultiplayerMenuWidget::HandleBackClicked);
	}
}

void UMultiplayerMenuWidget::HandleBackClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APC_Menu* MenuPC = Cast<APC_Menu>(PC))
		{
			MenuPC->ShowMainMenu();
		}
	}
}