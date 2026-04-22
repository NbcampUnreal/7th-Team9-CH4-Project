#include "SpikeRoller.h"

#include "Net/UnrealNetwork.h"

ASpikeRoller::ASpikeRoller()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);
    
    RollerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RollerMesh"));
    RootComponent = RollerMesh;
}

void ASpikeRoller::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASpikeRoller, ElapsedTime);
}

void ASpikeRoller::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (HasAuthority())
    {
        ElapsedTime += DeltaTime;
    }
    
    float Direction = bReverseRotation ? -1.0f : 1.0f;
    float TotalRotation = ElapsedTime * RotationSpeed * Direction;
    SetActorRotation(FRotator(0.f, TotalRotation, 0.f));
}