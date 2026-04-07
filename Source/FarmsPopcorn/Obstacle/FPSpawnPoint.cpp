#include "obstacle/FPSpawnPoint.h"

#include "Components/BoxComponent.h"

AFPSpawnPoint::AFPSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	RootComponent = BoxCollision;
	BoxCollision->SetCollisionProfileName(TEXT("Trigger"));
}


