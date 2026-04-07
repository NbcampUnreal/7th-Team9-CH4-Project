#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSpawnPoint.generated.h"

UCLASS()
class FARMSPOPCORN_API AFPSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	AFPSpawnPoint();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<class UBoxComponent> BoxCollision;
};
