// FloorSwitch.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloorSwitch.generated.h"

class AMovingPlatform; // 전방 선언

UCLASS()
class FARMSPOPCORN_API AFloorSwitch : public AActor
{
    GENERATED_BODY()

public:
    AFloorSwitch();
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    USceneComponent* RootScene;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* SwitchMesh;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* TriggerBox;

    // 언리얼 에디터에서 이 스위치와 연결될 발판들을 지정할 수 있습니다.
    UPROPERTY(EditInstanceOnly, Category = "Settings|Target")
    TArray<AMovingPlatform*> TargetPlatforms;

    UPROPERTY(EditAnywhere, Category = "Settings|Switch")
    float SwitchLowerDepth = 15.0f;

    UPROPERTY(EditAnywhere, Category = "Settings")
    float LerpSpeed = 5.0f;

private:
    FVector StartPos;
    FVector TargetPos;
    bool bIsPressed = false;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};