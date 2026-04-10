#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlinkingPlatform.generated.h"

UCLASS()
class FARMSPOPCORN_API ABlinkingPlatform : public AActor
{
    GENERATED_BODY()

public:
    ABlinkingPlatform();
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* PlatformMesh;

    // 발판이 보이고/사라지는 주기 (초)
    UPROPERTY(EditAnywhere, Category = "Settings")
    float Interval = 3.0f;

private:
    float Timer = 0.0f;
    bool bIsVisible = true;

    // 상태 업데이트 함수
    void UpdatePlatformState();
};