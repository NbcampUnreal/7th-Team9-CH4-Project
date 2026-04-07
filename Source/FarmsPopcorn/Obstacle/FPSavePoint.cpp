#include "obstacle/FPSavePoint.h"

#include "Components/BoxComponent.h"
#include "Player/FPPlayerCharacter.h"

AFPSavePoint::AFPSavePoint()
{
	PrimaryActorTick.bCanEverTick = false;
	
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	RootComponent = BoxCollision;
	BoxCollision->SetCollisionProfileName(TEXT("Trigger"));
}

void AFPSavePoint::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp,int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this))
	{
		if (AFPPlayerCharacter* Player = Cast<AFPPlayerCharacter>(OtherActor))
		{
			Player->SetSavePointLocation(GetActorLocation(), GetActorRotation());
		}
	}
}

void AFPSavePoint::BeginPlay()
{
	Super::BeginPlay();
	
	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AFPSavePoint::OnComponentBeginOverlap);
}
