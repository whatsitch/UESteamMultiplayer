#include "LobbyPlayerEntryWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void ULobbyPlayerEntryWidget::InitFromPlayerState(APlayerState* PlayerState, bool bIsHost)
{
	if (!PlayerState) return;

	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(PlayerState->GetPlayerName()));
	}

	if (RoleText)
	{
		if (bIsHost)
		{
			RoleText->SetText(FText::FromString(TEXT("Host")));
			RoleText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			RoleText->SetText(FText::GetEmpty());
			RoleText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}