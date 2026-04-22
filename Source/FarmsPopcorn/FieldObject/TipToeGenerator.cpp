// TipToeGenerator.cpp

#include "TipToeGenerator.h"
#include "FakeTile.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"

ATipToeGenerator::ATipToeGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    RealMeshComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RealMeshComp"));
    RootComponent = RealMeshComp;

    FakeMeshComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FakeMeshComp"));
    FakeMeshComp->SetupAttachment(RootComponent);

    // 기본값 설정 
    Rows = 15;
    Columns = 10;
    BlockSize = 200.f;
    Padding = 100.f;
    FakeChance = 0.7f;
    bAutoSizeToMesh = true;
}

int32 ATipToeGenerator::GetIndex(int32 X, int32 Y) const
{
    return X * Columns + Y;
}

void ATipToeGenerator::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ATipToeGenerator, GridData);
    DOREPLIFETIME(ATipToeGenerator, FinalInterval);
}

void ATipToeGenerator::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // 맵 재생성
    GenerateMaze();
}

void ATipToeGenerator::Multicast_SpawnFakeTiles_Implementation(float InFinalInterval)
{
    if (!FakeTileClass) return;

    // FakeMeshComp 인스턴스 대신 GridData에서 직접 위치 계산
    for (int32 X = 0; X < Rows; ++X)
    {
        for (int32 Y = 0; Y < Columns; ++Y)
        {
            if (!GridData[GetIndex(X, Y)]) // 가짜 블록만
            {
                FVector Location = FVector(X * InFinalInterval, Y * InFinalInterval, 0.f);
                FTransform T;
                T.SetLocation(Location);

                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = 
                    ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

                GetWorld()->SpawnActor<AFakeTile>(FakeTileClass, T, SpawnParams);
            }
        }
    }

    if (FakeMeshComp) FakeMeshComp->ClearInstances();
}

void ATipToeGenerator::GenerateMaze()
{
    float CurrentMeshSize = MeshSize;
    if (bAutoSizeToMesh && RealMeshComp && RealMeshComp->GetStaticMesh())
    {
        FBoxSphereBounds Bounds = RealMeshComp->GetStaticMesh()->GetBounds();
        CurrentMeshSize = Bounds.BoxExtent.X * 2.0f;
    }
    FinalInterval = CurrentMeshSize + Padding;

    // 기존에 생성된 블럭 초기화
    if (RealMeshComp) RealMeshComp->ClearInstances();
    if (FakeMeshComp) FakeMeshComp->ClearInstances();

    // 배열 전부 가짜 블록으로 초기화
    int32 TotalBlocks = Rows * Columns;
    GridData.Init(false, TotalBlocks);

    // 정답 경로 생성 (랜덤 워크 알고리즘)
    int32 CurrentX = 0;
    int32 CurrentY = FMath::RandRange(0, Columns - 1);
    GridData[GetIndex(CurrentX, CurrentY)] = true;

    // 도착 지점에 도착까지 길 생성
    while (CurrentX < Rows - 1)
    {
        // 50% 확률로 직진 또는 좌우 이동
        if (FMath::RandBool())
        {
            CurrentX++;
            GridData[GetIndex(CurrentX, CurrentY)] = true;
        }
        else
        {
            // 좌우 이동 목표지점 랜덤 선택
            int32 TargetY = FMath::RandRange(0, Columns - 1);
            int32 Step = (TargetY > CurrentY) ? 1 : -1;
            while (CurrentY != TargetY)
            {
                CurrentY += Step;
                GridData[GetIndex(CurrentX, CurrentY)] = true;
            }

            // 좌우 이동간 제자리 맴돌지 않게 한 칸 전진
            CurrentX++;
            GridData[GetIndex(CurrentX, CurrentY)] = true;
        }
    }

    // 블럭 배치 로직
    for (int32 X = 0; X < Rows; ++X)
    {
        for (int32 Y = 0; Y < Columns; ++Y)
        {
            int32 Index = GetIndex(X, Y);
            bool bIsReal = GridData[Index];

            if (!bIsReal)
            {
                bIsReal = (FMath::FRand() > FakeChance);
            }

            FVector Location = FVector(X * FinalInterval, Y * FinalInterval, 0.f);
            FTransform InstanceTransform;
            InstanceTransform.SetLocation(Location);

            if (bIsReal)
            {
                RealMeshComp->AddInstance(InstanceTransform);
            }
            else
            {
                FakeMeshComp->AddInstance(InstanceTransform);
            }
        }
    }
}

void ATipToeGenerator::OnRep_GridData()
{
    // GridData 기반으로 RealMeshComp 인스턴스 재구성
    if (RealMeshComp) RealMeshComp->ClearInstances();

    for (int32 X = 0; X < Rows; ++X)
    {
        for (int32 Y = 0; Y < Columns; ++Y)
        {
            if (GridData[GetIndex(X, Y)]) // 진짜 블록만
            {
                FVector Location = FVector(X * FinalInterval, Y * FinalInterval, 0.f);
                FTransform T;
                T.SetLocation(Location);
                RealMeshComp->AddInstance(T);
            }
        }
    }
}

void ATipToeGenerator::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        // 매 실행시 발판 위치 재생성
        GenerateMaze();
        Multicast_SpawnFakeTiles(FinalInterval);
    }
}