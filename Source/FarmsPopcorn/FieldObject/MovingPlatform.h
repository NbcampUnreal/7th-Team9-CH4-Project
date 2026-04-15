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

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* PlatformMesh;

    // --- 에디터 설정 변수 ---

    // 목표 지점 오프셋 (에디터에서 화살표나 수치로 직접 조정 가능)
    // 예: (500, 0, 0)이면 X축으로 500만큼 갔다가 돌아옴
    UPROPERTY(EditAnywhere, Category = "Settings", meta = (MakeEditWidget = true))
    FVector TargetOffset = FVector(0.0f, 0.0f, 500.0f);

    // 이동 속도 (0.0 ~ 1.0 사이를 왕복하는 속도)
    UPROPERTY(EditAnywhere, Category = "Settings")
    float MoveSpeed = 1.0f;

private:
    FVector StartLocation;
    FVector TargetLocation;
};