#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPObstacleBase.generated.h"

UCLASS()
class FARMSPOPCORN_API AFPObstacleBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AFPObstacleBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
