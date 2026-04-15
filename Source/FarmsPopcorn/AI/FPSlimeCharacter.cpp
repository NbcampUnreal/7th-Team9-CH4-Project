#include "AI/FPSlimeCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AFPSlimeCharacter::AFPSlimeCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(RootComponent);
}

void AFPSlimeCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
	
	//동적 머티리얼 생성
	if (StaticMesh && StaticMesh->GetMaterial(0))
	{
		DynamicMaterial = StaticMesh->CreateAndSetMaterialInstanceDynamic(0);
	}
}


void AFPSlimeCharacter::Multicast_SetBlinkRed_Implementation(bool bBlink)
{
	if (DynamicMaterial)
	{
		float Alpha = bBlink ? 0.5f : 0.0f;
		DynamicMaterial->SetScalarParameterValue("BlinkAlpha", Alpha);
		DynamicMaterial->SetVectorParameterValue("BlinkColor", FLinearColor::Red);
	}
}

void AFPSlimeCharacter::SetBlinkRed(bool bBlink)
{
	if (!HasAuthority()) return;
	
	Multicast_SetBlinkRed(bBlink);
}

void AFPSlimeCharacter::ApplyKnockback(ACharacter* Target)
{
	if (!HasAuthority()) return;
	
	if (!Target) return;
	
	FVector LaunchDir = Target->GetActorLocation() - GetActorLocation();
	LaunchDir.Normalize();
	LaunchDir.Z = FMath::Max(LaunchDir.Z, 0.5f);
	
	FVector FinalVelocity = LaunchDir * Knockback;
	Target->LaunchCharacter(FinalVelocity, true, true);
}