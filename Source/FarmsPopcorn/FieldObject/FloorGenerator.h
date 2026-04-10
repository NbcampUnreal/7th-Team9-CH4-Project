#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloorGenerator.generated.h"

UCLASS()
class FARMSPOPCORN_API AFloorGenerator : public AActor
{
    GENERATED_BODY()

public:
    AFloorGenerator();

    virtual void OnConstruction(const FTransform& Transform) override;

protected:
    // ISM 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UInstancedStaticMeshComponent* InstancedMeshComp;

    // 가로 블록 개수
    UPROPERTY(EditAnywhere, Category = "Grid Settings")
    int32 Rows;

    // 세로 블록 개수
    UPROPERTY(EditAnywhere, Category = "Grid Settings")
    int32 Columns;

    // 블록의 크기 및 간격
    UPROPERTY(EditAnywhere, Category = "Grid Settings")
    float BlockSize;
};