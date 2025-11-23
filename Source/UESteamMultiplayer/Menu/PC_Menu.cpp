#include "Menu/PC_Menu.h"
#include "UI/Main/MainMenuWidget.h"
#include "Blueprint/UserWidget.h"

void APC_Menu::BeginPlay()
{
	Super::BeginPlay();

	// Debug: Prüfen, ob BeginPlay überhaupt aufgerufen wird
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("APC_Menu::BeginPlay"));
	}

	bShowMouseCursor = true;

	if (!MainMenuWidgetClass)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("MainMenu widget not set"));
		return;
	}

	MainMenuWidget = CreateWidget<UMainMenuWidget>(this, MainMenuWidgetClass);
	if (MainMenuWidget)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Widget created"));
		MainMenuWidget->AddToViewport();

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("CreateWidget failed"));
	}
}
