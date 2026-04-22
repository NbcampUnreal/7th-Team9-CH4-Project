// Flipper.h

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
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* FlipperMesh;

    UPROPERTY(EditAnywhere, Category = "Settings")
    float MaxRotationAngle = -90.0f;

    UPROPERTY(EditAnywhere, Category = "Settings")
    float OutSpeed = 0.15f;

    UPROPERTY(EditAnywhere, Category = "Settings")
    float StayTime = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Settings")
    float InSpeed = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Settings")
    float CycleInterval = 4.0f;

    UPROPERTY(EditAnywhere, Category = "Settings")
    float BaseLaunchStrength = 4000.0f;

    UPROPERTY(EditAnywhere, Category = "Settings")
    float StartOffset = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Flipper Settings")
    FVector RotationAxis = FVector(1.0f, 0.0f, 0.0f);  

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class USceneComponent* PivotScene;
     

private:
    float ElapsedTime = 0.0f;

    // 날리기 로직 통합
    void LaunchTarget(AActor* OtherActor);

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
    virtual void BeginPlay() override;
};