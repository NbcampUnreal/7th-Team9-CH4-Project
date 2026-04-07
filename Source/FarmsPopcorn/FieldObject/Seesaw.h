#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SeeSaw.generated.h"

class UBoxComponent;

UCLASS()
class FARMSPOPCORN_API ASeeSaw : public AActor
{
    GENERATED_BODY()

public:
    ASeeSaw();
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* PlankMesh;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UBoxComponent* DetectionBox;

    // --- 에디터 수정 가능 변수들 ---

    // 시소 중심에서 이 거리(단위) 이내에 있으면 움직이지 않음
    UPROPERTY(EditAnywhere, Category = "Settings")
    float DeadzoneRadius = 60.0f;

    // 무게 감도 (누적 속도)
    UPROPERTY(EditAnywhere, Category = "Settings")
    float WeightSensitivity = 2.0f;

    // 최대 기울기 각도
    UPROPERTY(EditAnywhere, Category = "Settings")
    float MaxTiltAngle = 60.0f;

    // 복원 속도
    UPROPERTY(EditAnywhere, Category = "Settings")
    float RecoverySpeed = 1.5f;

private:
    float CurrentTilt = 0.0f;
};