#include "Menu/PC_Menu.h"
#include "UI/Menu/Main/MainMenuWidget.h"
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

	ShowMainMenu();
}

void APC_Menu::ShowMainMenu()
{
	ShowMenuWidget(MainMenuWidgetClass);
}

void APC_Menu::ShowMultiplayerMenu()
{
	ShowMenuWidget(MultiplayerMenuWidgetClass);
}

void APC_Menu::ShowMenuWidget(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass) return;

	/*----- remove old menu -----*/
	if (CurrentMenuWidget)
	{
		CurrentMenuWidget->RemoveFromParent();
		CurrentMenuWidget = nullptr;
	}

	/*----- add new menu -----*/
	CurrentMenuWidget = CreateWidget<UUserWidget>(this, WidgetClass);
	if (!CurrentMenuWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("PC_Menu: CreateWidget failed"));
		return;
	}

	CurrentMenuWidget->AddToViewport();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(CurrentMenuWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}
