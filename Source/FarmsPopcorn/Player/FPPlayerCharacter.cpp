#include "Player/FPPlayerCharacter.h"

AFPPlayerCharacter::AFPPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

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

