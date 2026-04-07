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

		
	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp,AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult);
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<class UBoxComponent> BoxCollision;
};
