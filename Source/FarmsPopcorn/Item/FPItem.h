// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/FPPlayerCharacter.h" // EItemType 사용을 위해 필요
#include "FPItem.generated.h"

UCLASS()
class FARMSPOPCORN_API AFPItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 충돌 처리 함수
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	// 아이템 상자 재생성 함수
	void RespawnItem();

protected:
	// 컴포넌트들
	UPROPERTY(VisibleAnywhere, Category = "Item|Components")
	TObjectPtr<class UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere, Category = "Item|Components")
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	// 설정값
	UPROPERTY(EditAnywhere, Category = "Item|Settings")
	float RespawnTime = 5.0f; // 아이템 재생성 시간

	FTimerHandle RespawnTimerHandle;

};
