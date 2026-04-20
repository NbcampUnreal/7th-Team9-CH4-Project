#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpikeRoller.generated.h"

UCLASS()
class FARMSPOPCORN_API ASpikeRoller : public AActor
{
    GENERATED_BODY()

public:
    ASpikeRoller();
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* RollerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float RotationSpeed = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bReverseRotation = false;
};