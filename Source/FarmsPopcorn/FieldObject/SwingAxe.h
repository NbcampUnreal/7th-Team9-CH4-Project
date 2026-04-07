#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SwingAxe.generated.h"

UCLASS()
class FARMSPOPCORN_API ASwingAxe : public AActor
{
    GENERATED_BODY()

public:
    ASwingAxe();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* AxeMesh;

    UPROPERTY(EditAnywhere, Category = "Settings")
    float MaxSwingAngle = 60.0f;

    UPROPERTY(EditAnywhere, Category = "Settings")
    float SwingSpeed = 2.0f;

    // 너무 멀리 날아간다면 에디터에서 이 값을 1000~1200 정도로 낮추세요.
    UPROPERTY(EditAnywhere, Category = "Settings")
    float LaunchStrength = 1200.0f;

private:
    // 이미지에 있던 변수
    float RunningTime = 0.0f;

    UFUNCTION()
    void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};