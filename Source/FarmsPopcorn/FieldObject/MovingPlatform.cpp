#include "MovingPlatform.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

AMovingPlatform::AMovingPlatform()
{
    PrimaryActorTick.bCanEverTick = true;

    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = RootScene;

    PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
    PlatformMesh->SetupAttachment(RootScene);
}

void AMovingPlatform::BeginPlay()
{
    Super::BeginPlay();

    StartPos = PlatformMesh->GetRelativeLocation();
    TargetPos = StartPos + FVector(0.0f, 0.0f, MaxHeight);
}

void AMovingPlatform::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector CurrentPos = PlatformMesh->GetRelativeLocation();
    FVector Target = bIsActivated ? TargetPos : StartPos;

    PlatformMesh->SetRelativeLocation(FMath::VInterpTo(CurrentPos, Target, DeltaTime, LerpSpeed));
}

void AMovingPlatform::SetPlatformActive(bool bActive)
{
    bIsActivated = bActive;
}