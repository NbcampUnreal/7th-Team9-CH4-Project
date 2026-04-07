#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include  "Core/FPTeamID.h"
#include "FPPlayerState.generated.h"

UCLASS()
class FARMSPOPCORN_API AFPPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AFPPlayerState();
	bool bIsReady = false;
	void Server_SetReady_Implementation(bool bNewReadyState);
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Team")
	EFPTeamID TeamID = EFPTeamID::None;
#pragma region Name //캐릭터 이름 설정
	
	UFUNCTION()
	void OnRep_CustomPlayerName();

	UPROPERTY(ReplicatedUsing = OnRep_CustomPlayerName)
	FString CustomPlayerName;
#pragma endregion Name	//캐릭터 이름 설정 끝

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
