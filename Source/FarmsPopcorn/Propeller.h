#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Propeller.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class URotatingMovementComponent;

UCLASS()
class FARMSPOPCORN_API APropeller : public AActor
{
    GENERATED_BODY()
public:
    APropeller();

protected:
    virtual void BeginPlay() override;

public:
    // 프로펠러의 외형
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Propeller|Components")
    UStaticMeshComponent* PropellerMesh;

    // 캐릭터와의 충돌 감지 스피어 콜리전
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Propeller|Components")
    USphereComponent* CollisionComponent;

    // 속도
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Propeller|Components")
    URotatingMovementComponent* RotatingComponent;

    // 밀어내는 넉백의 세기
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Propeller|Knockback")
    float KnockbackStrength;

    // 맞았을 때 공중으로 뜨게 하는 힘
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Propeller|Knockback")
    float ZAxisLaunchVelocity;

private:
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};