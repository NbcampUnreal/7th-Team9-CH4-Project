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
void ASpikeRoller::BeginPlay()
{
    Super::BeginPlay();
    // 에디터에 배치된 시점의 회전값을 저장
    InitialRotation = GetActorRotation();
}

void ASpikeRoller::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASpikeRoller, ElapsedTime);
}

void ASpikeRoller::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    float Direction = bReverseRotation ? -1.0f : 1.0f;
    float RotationDelta = RotationSpeed * Direction * DeltaTime;
    
    AddActorLocalRotation(FRotator(0.f, RotationDelta, 0.f));
}