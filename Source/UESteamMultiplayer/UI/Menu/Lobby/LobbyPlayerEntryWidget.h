#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "LobbyPlayerEntryWidget.generated.h"

/**
 * 
 */
UCLASS()
class UESTEAMMULTIPLAYER_API ULobbyPlayerEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta=(BindWidget))
	UTextBlock* PlayerNameText;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* RoleText; // z.B. "Host"

public:
	UFUNCTION(BlueprintCallable, Category="Lobby")
	void InitFromPlayerState(APlayerState* PlayerState, bool bIsHost);
	
};
