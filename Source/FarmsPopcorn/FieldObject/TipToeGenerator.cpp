// TipToeGenerator.cpp

#include "TipToeGenerator.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Math/UnrealMathUtility.h"

ATipToeGenerator::ATipToeGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    RealMeshComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RealMeshComp"));
    RootComponent = RealMeshComp;

    FakeMeshComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FakeMeshComp"));
    FakeMeshComp->SetupAttachment(RootComponent);

    // 기본값 설정 
    Rows = 15;
    Columns = 10;
    BlockSize = 200.f;
    FakeChance = 0.7f;
}

int32 ATipToeGenerator::GetIndex(int32 X, int32 Y) const
{
    return X * Columns + Y;
}

void ATipToeGenerator::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // 매 판 맵 재생성
    GenerateMaze();
}

void ATipToeGenerator::GenerateMaze()
{
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

    // 도착 지점(Rows - 1)에 도착까지 길 생성
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

            // 블럭 사이즈에 따라 블럭 위치 계산
            FVector InstanceLocation = FVector(X * BlockSize, Y * BlockSize, 0.f);
            FTransform InstanceTransform;
            InstanceTransform.SetLocation(InstanceLocation);

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