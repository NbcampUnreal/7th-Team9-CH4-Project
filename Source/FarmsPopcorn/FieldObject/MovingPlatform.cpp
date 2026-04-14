#include "MovingPlatform.h"

AMovingPlatform::AMovingPlatform()
{
    PrimaryActorTick.bCanEverTick = true;

    PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
    RootComponent = PlatformMesh;
}

void AMovingPlatform::BeginPlay()
{
    Super::BeginPlay();

    StartLocation = GetActorLocation();

    // 선택한 Enum에 따라 이동 축 벡터 설정
    switch (MoveDirection)
    {
    case EMoveDirection::UpDown:
        MoveAxis = FVector(0.0f, 0.0f, 1.0f);
        break;
    case EMoveDirection::LeftRight:
        MoveAxis = FVector(0.0f, 1.0f, 0.0f);
        break;
    case EMoveDirection::ForwardBack:
        MoveAxis = FVector(1.0f, 0.0f, 0.0f);
        break;
    }
}

void AMovingPlatform::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 시간에 따라 -1.0 ~ 1.0 사이를 반복하는 값 계산
    // RunningTime * Speed / Distance 로 주기를 조절
    float Time = GetGameTimeSinceCreation();
    float Movement = FMath::Sin(Time * (Speed / Distance) * 2.0f);

    // 시작 위치에서 축 방향으로 설정한 거리만큼 이동
    FVector NewLocation = StartLocation + (MoveAxis * Movement * Distance);

    SetActorLocation(NewLocation);
}