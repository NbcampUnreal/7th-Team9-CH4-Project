#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MovingPlatform.generated.h"

UCLASS()
class FARMSPOPCORN_API AMovingPlatform : public AActor
{
    GENERATED_BODY()

public:
    AMovingPlatform();
    virtual void Tick(float DeltaTime) override;

    // 스위치에서 호출할 함수 선언
    void SetPlatformActive(bool bActive);

protected:
    virtual void BeginPlay() override;

    // 컴포넌트 선언
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class USceneComponent* RootScene;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* PlatformMesh;

    UPROPERTY(EditAnywhere, Category = "Settings|Mode")
    bool bIsAutomatic = true;

    // 에디터 설정 변수 선언
    UPROPERTY(EditAnywhere, Category = "Settings")
    float MaxHeight = 400.0f;

    UPROPERTY(EditAnywhere, Category = "Settings")
    float LerpSpeed = 5.0f;

private:
    // 내부 위치 계산용 변수 선언
    FVector StartPos;
    FVector TargetPos;

    bool bIsActivated = false;
    bool bMovingToTarget = true;
};