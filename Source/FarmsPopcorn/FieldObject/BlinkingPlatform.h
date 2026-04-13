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

    void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
    
protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* PlatformMesh;

    // 발판이 보이고/사라지는 주기 (초)
    UPROPERTY(EditAnywhere, Category = "Settings")
    float Interval = 3.0f;

    UPROPERTY(ReplicatedUsing = OnRep_IsVisible)
    bool bIsVisible = true;
    
    UFUNCTION()
    void OnRep_IsVisible();

private:
    float Timer = 0.0f;
    // 상태 업데이트 함수
    void UpdatePlatformState();
};