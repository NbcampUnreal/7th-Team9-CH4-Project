#include "Propeller.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "GameFramework/Character.h"

APropeller::APropeller()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    
    // 메시 컴포넌트 초기화
    PropellerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PropellerMesh"));
    RootComponent = PropellerMesh;

    PropellerMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    PropellerMesh->SetGenerateOverlapEvents(false);

    // 콜리전 컴포넌트 초기화
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetupAttachment(RootComponent);

    CollisionComponent->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
    CollisionComponent->SetGenerateOverlapEvents(true);

    // 회전 컴포넌트
    RotatingComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingComponent"));
    RotatingComponent->RotationRate = FRotator(0.f, 360.f, 0.f);

    KnockbackStrength = 1500.f;
    ZAxisLaunchVelocity = 600.f;
    SafeHeightThreshold = 50.f;
}

void APropeller::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority())
    {
        return;
    }
    
    if (CollisionComponent)
    {
        CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &APropeller::OnOverlapBegin);
    }
}

void APropeller::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this)
    {
        ACharacter* HitCharacter = Cast<ACharacter>(OtherActor);

        if (HitCharacter)
        {
            float CharacterFootZ = HitCharacter->GetActorLocation().Z - HitCharacter->GetSimpleCollisionHalfHeight();
            float PropellerZ = GetActorLocation().Z;

            if (CharacterFootZ > PropellerZ + SafeHeightThreshold)
            {
                return;
            }
           
            FVector LaunchDirection = HitCharacter->GetActorLocation() - GetActorLocation();
            LaunchDirection.Z = 0.f;
            LaunchDirection.Normalize();

            FVector LaunchVelocity = LaunchDirection * KnockbackStrength;
            LaunchVelocity.Z += ZAxisLaunchVelocity;

            HitCharacter->LaunchCharacter(LaunchVelocity, true, true);
        }
    }
}