#include "SpikeRoller.h"

ASpikeRoller::ASpikeRoller()
{
    PrimaryActorTick.bCanEverTick = true;

    RollerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RollerMesh"));
    RootComponent = RollerMesh;
}

void ASpikeRoller::BeginPlay()
{
    Super::BeginPlay();
}

void ASpikeRoller::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float Direction = bReverseRotation ? -1.0f : 1.0f;

    float RotationAmount = RotationSpeed * Direction * DeltaTime;

    AddActorLocalRotation(FRotator(0.0f, RotationAmount, 0.0f));
}