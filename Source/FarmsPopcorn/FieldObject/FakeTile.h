// FakeTile.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FakeTile.generated.h"

UCLASS()
class FARMSPOPCORN_API AFakeTile : public AActor
{
    GENERATED_BODY()

public:
    AFakeTile();

protected:
    virtual void BeginPlay() override;

    // 발판 외형
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* TileMesh;

    // 밟았는지 감지할 센서
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* TriggerBox;

    // 밟고 나서 떨어지기까지의 지연 시간
    UPROPERTY(EditAnywhere, Category = "Tile Settings")
    float FallDelay;

    // 타이머 핸들
    FTimerHandle FallTimerHandle;

    UFUNCTION()
    void OnPlayerStepped(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // 발판을 떨어뜨리는 함수
    void DropTile();
};