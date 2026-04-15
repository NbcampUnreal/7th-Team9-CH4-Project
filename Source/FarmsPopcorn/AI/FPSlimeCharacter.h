#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSlimeCharacter.generated.h"

UCLASS()
class FARMSPOPCORN_API AFPSlimeCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFPSlimeCharacter();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, Category = "StaticMesh")
	TObjectPtr<class UStaticMeshComponent> StaticMesh;
	
	// 플레이어 감지 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	float DetectionRadius = 2000.f;
	
	// 돌진 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	float ChargeSpeed = 3000.f;
	
	// 넉백 세기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	FVector Knockback = FVector(500.f, 500.f, 200.f);
	
	//순찰 이동속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	float PatrolSpeed = 300.f;
	
	// 머티리얼 깜빡임용
	UPROPERTY(VisibleAnywhere, Category = "Material Blink")
	TObjectPtr<class UMaterialInstanceDynamic> DynamicMaterial;
	
public:
	//BT에서 쓸 getter
	float GetDetectionRadius() const { return DetectionRadius; }
	float GetChargeSpeed() const { return ChargeSpeed; }
	FVector GetKnockback() const { return Knockback; }
	float GetPatrolSpeed() const { return PatrolSpeed; }
	
	//머티리얼을 빨간색으로
	void SetBlinkRed(bool bBlink);
	
	// 서버에서 클라이언트로
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetBlinkRed(bool bBlink);
	
	//넉백 계산
	void ApplyKnockback(ACharacter* Target);
};
