#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/Main/MainMenuWidget.h"
#include "PC_Menu.generated.h"


UCLASS()
class UESTEAMMULTIPLAYER_API APC_Menu : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UMainMenuWidget> MainMenuWidgetClass;

private:
	UPROPERTY()
	UMainMenuWidget* MainMenuWidget = nullptr;
};
