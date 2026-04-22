// FakeTile.cpp

#include "FakeTile.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

AFakeTile::AFakeTile()
{
    PrimaryActorTick.bCanEverTick = false;

    // 메쉬 설정
    TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
    RootComponent = TileMesh;

    // 트리거 박스 설정
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetupAttachment(RootComponent);
    TriggerBox->SetBoxExtent(FVector(90.f, 90.f, 20.f)); 
    TriggerBox->SetRelativeLocation(FVector(0.f, 0.f, 100.f));

    FallDelay = 0.3f; 
}

void AFakeTile::BeginPlay()
{
    Super::BeginPlay();

    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AFakeTile::OnPlayerStepped);
}

void AFakeTile::OnPlayerStepped(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
    {
        GetWorldTimerManager().SetTimer(FallTimerHandle, this, &AFakeTile::DropTile, FallDelay, false);
    }
}

void AFakeTile::DropTile()
{
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("DropTile Executed! Physics ON."));

    TileMesh->SetSimulatePhysics(true);

    SetLifeSpan(3.0f);
}