#include "Player/FPPlayerCharacter.h"

AFPPlayerCharacter::AFPPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

}

void AFPPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AFPPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFPPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

