// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"        // 데이터 테이블을 쓰기 위해 반드시 추가!
#include "Player/FPPlayerCharacter.h" // EItemType 사용을 위해 필요
#include "FPItem.generated.h"

// ----------------------------------------------------
// 데이터 테이블용 구조체 선언 (FTableRowBase 상속 필수)
// ----------------------------------------------------
USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 1. 아이템 종류
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemData")
	EItemType ItemType;

	// 2. 효과 지속 시간 (물풍선 기절 2초, 고구마 버프 5초 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemData")
	float Duration;

	// 3. 아이템 위력 (선풍기가 밀어내는 힘 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemData")
	float EffectPower;

	// 4. 등장 확률 가중치 (이 값이 높을수록 박스에서 잘 나옴)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemData")
	int32 SpawnWeight;
};

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

	// 에디터에서 방금 만든 데이터 테이블을 넣을 빈칸
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Data")
	UDataTable* ItemDataTable;
};
