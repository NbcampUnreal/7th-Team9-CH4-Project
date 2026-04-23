// SwitchPlatform.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SwitchPlatform.generated.h"

UCLASS()
class FARMSPOPCORN_API ASwitchPlatform : public AActor
{
    GENERATED_BODY()

public:
    ASwitchPlatform();
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

    // --- 컴포넌트 ---
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USceneComponent* RootScene;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* SwitchMesh; // 밟는 버튼

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* TargetPlatformMesh; // 상승하는 발판

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* TriggerBox; // 플레이어 감지용

    // --- 에디터 설정 변수 ---
    UPROPERTY(EditAnywhere, Category = "Settings|Switch")
    float SwitchLowerDepth = 15.0f; // 버튼이 눌리는 깊이

    UPROPERTY(EditAnywhere, Category = "Settings|Platform")
    float PlatformRiseHeight = 400.0f; // 발판이 상승할 높이

    UPROPERTY(EditAnywhere, Category = "Settings")
    float LerpSpeed = 5.0f; // 움직임의 부드러운 속도

private:
    FVector SwitchStartPos;
    FVector SwitchTargetPos;
    FVector PlatformStartPos;
    FVector PlatformTargetPos;

    bool bIsPressed = false;

    // 충돌 이벤트 함수
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};