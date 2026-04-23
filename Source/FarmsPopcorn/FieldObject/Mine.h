// Mine.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Mine.generated.h"

UCLASS()
class FARMSPOPCORN_API AMine : public AActor
{
    GENERATED_BODY()

public:
    AMine();

protected:
    virtual void BeginPlay() override;

    // 메쉬
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* MineMesh;

    // 캐릭터 접근 감지 
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class USphereComponent* DetectionSphere;

    // 지뢰 활성화 시간 조절
    UPROPERTY(EditAnywhere, Category = "Mine Settings")
    float ActivationDelay;

    // 밟은 후 폭발까지의 시간
    UPROPERTY(EditAnywhere, Category = "Mine Settings")
    float ExplosionDelay;

    // 폭발 반경
    UPROPERTY(EditAnywhere, Category = "Mine Settings")
    float ExplosionRadius;

    // 넉백 세기
    UPROPERTY(EditAnywhere, Category = "Mine Settings")
    FVector KnockbackForce;

    bool bIsActive;
    bool bHasBeenTriggered;

    FTimerHandle ActivationTimerHandle;
    FTimerHandle ExplosionTimerHandle;

    void ActivateMine();

    UFUNCTION()
    void OnCharacterDetected(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    void Pop();
    
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_Pop();
};