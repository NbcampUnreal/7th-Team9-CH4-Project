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

    virtual void BeginPlay() override;

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

    // 블록의 크기
    UPROPERTY(EditAnywhere, Category = "TipToe Settings")
    float MeshSize;

    // 블록 간격
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TipToe Settings")
    float Padding;

    // 블록 사이즈 자동 측정
    UPROPERTY(EditAnywhere, Category = "TipToe Settings")
    bool bAutoSizeToMesh;

    // 최종 간격
    UPROPERTY(Replicated)
    float FinalInterval;

    // 블록 한 칸의 크기 조절
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TipToe Settings")
    float BlockSize;

    // 가짜 발판이 섞일 확률 (0.0 ~ 1.0)
    UPROPERTY(EditAnywhere, Category = "TipToe Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FakeChance;

    // FakeTile로 가자발판 대체
    UPROPERTY(EditAnywhere, Category = "TipToe Settings")
    TSubclassOf<class AFakeTile> FakeTileClass;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_SpawnFakeTiles(float InFinalInterval);
    
    UPROPERTY(ReplicatedUsing = OnRep_GridData)
    TArray<bool> GridData;
    
    UFUNCTION()
    void OnRep_GridData();
    
    void GenerateMaze();
    int32 GetIndex(int32 X, int32 Y) const;
    
    void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};

