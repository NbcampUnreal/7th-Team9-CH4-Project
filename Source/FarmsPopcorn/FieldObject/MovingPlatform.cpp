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

    // 시작 위치 저장
    StartLocation = GetActorLocation();
    // 목표 위치 = 시작 위치 + 에디터에서 설정한 오프셋
    TargetLocation = StartLocation + TargetOffset;
}

void AMovingPlatform::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float Time = GetGameTimeSinceCreation() * MoveSpeed;

    float Alpha = (FMath::Sin(Time) * 0.5f) + 0.5f;

    FVector CurrentLocation = FMath::Lerp(StartLocation, TargetLocation, Alpha);
    SetActorLocation(CurrentLocation);
}