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
	bool bIsReady = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Team")
	EFPTeamID TeamID = EFPTeamID::None;
	
};
