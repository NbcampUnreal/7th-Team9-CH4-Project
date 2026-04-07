#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSavePoint.generated.h"

UCLASS()
class FARMSPOPCORN_API AFPSavePoint : public AActor
{
	GENERATED_BODY()
	
public:	
	AFPSavePoint();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<class UBoxComponent> BoxCollision;
};
