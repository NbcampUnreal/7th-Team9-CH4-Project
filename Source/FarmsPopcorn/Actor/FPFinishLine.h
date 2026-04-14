// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/FPTeamID.h"
#include "FPFinishLine.generated.h"

class UBoxComponent;
UCLASS()
class FARMSPOPCORN_API AFPFinishLine : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFPFinishLine();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//트리거 콜리전 박스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionBox;

	// 트리거 진입 시 호출될 함수
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						bool bFromSweep, const FHitResult& SweepResult);
	
};
