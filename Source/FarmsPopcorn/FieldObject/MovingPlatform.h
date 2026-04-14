#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MovingPlatform.generated.h"

UENUM(BlueprintType)
enum class EMoveDirection : uint8
{
    UpDown    UMETA(DisplayName = "Up Down"),
    LeftRight UMETA(DisplayName = "Left Right"),
    ForwardBack UMETA(DisplayName = "Forward Back")
};

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

    // 이동 방향 선택 (위아래 / 좌우 / 앞뒤)
    UPROPERTY(EditAnywhere, Category = "Settings")
    EMoveDirection MoveDirection = EMoveDirection::UpDown;

    // 이동 속도
    UPROPERTY(EditAnywhere, Category = "Settings")
    float Speed = 200.0f;

    // 이동 거리
    UPROPERTY(EditAnywhere, Category = "Settings")
    float Distance = 500.0f;

private:
    FVector StartLocation;
    FVector MoveAxis; // 실제 이동할 축 벡터
};