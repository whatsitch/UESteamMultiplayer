#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LobbyGameState.generated.h"


DECLARE_MULTICAST_DELEGATE(FLobbyPlayersChangedSignature);


UCLASS()
class UESTEAMMULTIPLAYER_API ALobbyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	/*----- Host PlayerState -----*/
	UPROPERTY(ReplicatedUsing = OnRep_HostPlayerState, BlueprintReadOnly, Category="Lobby")
	APlayerState* HostPlayerState = nullptr;

	FLobbyPlayersChangedSignature OnLobbyPlayersChanged;

	/*----- register replication -----*/
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

protected:
	UFUNCTION()
	void OnRep_HostPlayerState();
};
