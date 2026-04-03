#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Flipper.generated.h"

UCLASS()
class FARMSPOPCORN_API AFlipper : public AActor
{
    GENERATED_BODY()

public:
    AFlipper();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* FlipperMesh;

    // 설정값
    UPROPERTY(EditAnywhere, Category = "Settings")
    float MaxRotationAngle = 90.0f;

    UPROPERTY(EditAnywhere, Category = "Settings")
    float OutSpeed = 0.15f;  // 튀어나오는 속도 (0.15초)

    UPROPERTY(EditAnywhere, Category = "Settings")
    float StayTime = 0.5f;   // 중간에 멈춰있을 시간 (0.5초)

    UPROPERTY(EditAnywhere, Category = "Settings")
    float InSpeed = 1.0f;    // 다시 들어가는 속도 (1초)

    UPROPERTY(EditAnywhere, Category = "Settings")
    float CycleInterval = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Settings")
    float LaunchStrength = 3000.0f;

private:
    float ElapsedTime = 0.0f;

    UFUNCTION()
    void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};