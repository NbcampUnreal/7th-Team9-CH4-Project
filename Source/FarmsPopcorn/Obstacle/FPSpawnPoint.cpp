#include "obstacle/FPSpawnPoint.h"

#include "Components/BoxComponent.h"
#include "Player/FPPlayerCharacter.h"

AFPSpawnPoint::AFPSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	RootComponent = BoxCollision;
	BoxCollision->SetCollisionProfileName(TEXT("Trigger"));
}

void AFPSpawnPoint::OnOverLapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this))
	{
		if (AFPPlayerCharacter* Player = Cast<AFPPlayerCharacter>(OtherActor))
		{
			FVector TargetLocation = Player->GetSaveLocation();
			FRotator TargetRotation = Player->GetSaveRotation();
			
			Player->SetActorLocationAndRotation(TargetLocation, TargetRotation,false,
				nullptr, ETeleportType::TeleportPhysics);
			
			UPrimitiveComponent* CharacterMesh = Cast<UPrimitiveComponent>(Player->GetRootComponent());
			if (CharacterMesh && CharacterMesh->IsSimulatingPhysics())
			{
				CharacterMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
				CharacterMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
			}
		}
	}
}

void AFPSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AFPSpawnPoint::OnOverLapBegin);
}




