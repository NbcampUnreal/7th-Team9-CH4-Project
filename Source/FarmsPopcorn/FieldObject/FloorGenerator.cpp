#include "FloorGenerator.h"
#include "Components/InstancedStaticMeshComponent.h"

AFloorGenerator::AFloorGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    InstancedMeshComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedMeshComp"));
    RootComponent = InstancedMeshComp;

    Rows = 10;
    Columns = 10;
    BlockSize = 200.f; 
}

void AFloorGenerator::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (InstancedMeshComp)
    {
        InstancedMeshComp->ClearInstances();
    }

    for (int32 X = 0; X < Rows; ++X)
    {
        for (int32 Y = 0; Y < Columns; ++Y)
        {
            FVector InstanceLocation = FVector(X * BlockSize, Y * BlockSize, 0.f);

            FTransform InstanceTransform;
            InstanceTransform.SetLocation(InstanceLocation);

            InstancedMeshComp->AddInstance(InstanceTransform);
        }
    }
}