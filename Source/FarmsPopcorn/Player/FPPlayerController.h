#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPPlayerController.generated.h"

UCLASS()
class FARMSPOPCORN_API AFPPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Name")
	FString InName;
public:
	UFUNCTION(Server, Reliable)
	void ServerSetCustomName(const FString& NewName);
	
	UFUNCTION(Server, Reliable)
	void Server_SetReady(bool bNewReadyState);
	
	
};
