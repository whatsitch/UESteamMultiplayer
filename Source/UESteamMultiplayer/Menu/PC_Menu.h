#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/Menu/Main/MainMenuWidget.h"
#include "UI/Menu/Multiplayer/MultiplayerMenuWidget.h"
#include "PC_Menu.generated.h"


UCLASS()
class UESTEAMMULTIPLAYER_API APC_Menu : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Menu")
	void ShowMainMenu();

	UFUNCTION(BlueprintCallable, Category="Menu")
	void ShowMultiplayerMenu();

protected:
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UMainMenuWidget> MainMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UMultiplayerMenuWidget> MultiplayerMenuWidgetClass;

private:
	UPROPERTY()
	UUserWidget* CurrentMenuWidget = nullptr;

	void ShowMenuWidget(TSubclassOf<UUserWidget> WidgetClass);
};
