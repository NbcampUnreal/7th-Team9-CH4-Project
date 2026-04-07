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

	UFUNCTION()
	void OnOverLapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<class UBoxComponent> BoxCollision;
};
