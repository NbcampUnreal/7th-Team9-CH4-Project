// TipToeGenerator.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TipToeGenerator.generated.h"

UCLASS()
class FARMSPOPCORN_API ATipToeGenerator : public AActor
{
    GENERATED_BODY()

public:
    ATipToeGenerator();

    virtual void OnConstruction(const FTransform& Transform) override;

protected:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UInstancedStaticMeshComponent* RealMeshComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UInstancedStaticMeshComponent* FakeMeshComp;

    // X축 블록 개수
    UPROPERTY(EditAnywhere, Category = "TipToe Settings")
    int32 Rows;

    // Y축 블록 개수
    UPROPERTY(EditAnywhere, Category = "TipToe Settings")
    int32 Columns;

    // 블록 한 칸의 크기 조절
    UPROPERTY(EditAnywhere, Category = "TipToe Settings")
    float BlockSize;

    // 가짜 발판이 섞일 확률 (0.0 ~ 1.0)
    UPROPERTY(EditAnywhere, Category = "TipToe Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FakeChance;

    // FakeTile로 가자발판 대체
    UPROPERTY(EditAnywhere, Category = "TipToe Settings")
    TSubclassOf<class AFakeTile> FakeTileClass;

    TArray<bool> GridData;

    void GenerateMaze();
    int32 GetIndex(int32 X, int32 Y) const;
};