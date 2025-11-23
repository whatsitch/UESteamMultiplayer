#include "Menu/PC_Menu.h"

#include "SteamMultiplayerSubsystem.h"
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


	// Entscheide anhand der aktiven Session, welches Menü wir zeigen
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USteamMultiplayerSubsystem* Subsystem = GI->GetSubsystem<USteamMultiplayerSubsystem>())
		{
			if (Subsystem->HasActiveSession())
			{
				// Wir sind bereits in einer Lobby-Session → direkt Lobby anzeigen
				ShowMultiplayerMenu();
				return;
			}
		}
	}

	// Kein Multiplayer-Session aktiv → normales Hauptmenü
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
