#include "obstacle/FPSavePoint.h"

#include "Components/BoxComponent.h"

AFPSavePoint::AFPSavePoint()
{
	PrimaryActorTick.bCanEverTick = false;
	
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	RootComponent = BoxCollision;
	BoxCollision->SetCollisionProfileName(TEXT("Trigger"));
}
