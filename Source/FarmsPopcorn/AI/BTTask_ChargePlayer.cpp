#include "BTTask_ChargePlayer.h"

#include "AIController.h"
#include "AI/FPSlimeCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_ChargePlayer::UBTTask_ChargePlayer()
{
	NodeName = TEXT("ChargePlayer");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_ChargePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AFPSlimeCharacter* Slime = Cast<AFPSlimeCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Slime) return EBTNodeResult::Failed;
	
	ACharacter* Player = Cast<ACharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(PlayerKey.SelectedKeyName));
	if (!Player) return EBTNodeResult::Failed;
	
	Slime->GetCharacterMovement()->MaxWalkSpeed = Slime->GetChargeSpeed();
	
	ChargedDirection = Player->GetActorLocation() - Slime->GetActorLocation();
	ChargedDirection.Normalize();
	ChargedDistance = 0.f;
	
	return EBTNodeResult::InProgress;
}

void UBTTask_ChargePlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AFPSlimeCharacter* Slime = Cast<AFPSlimeCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Slime)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	FVector MoveAmount = ChargedDirection * Slime->GetChargeSpeed() * DeltaSeconds;
	Slime->AddMovementInput(ChargedDirection, 1.0f);
	ChargedDistance += MoveAmount.Size();
	
	TArray<AActor*> OverlappingActors;
	Slime->GetOverlappingActors(OverlappingActors,ACharacter::StaticClass());
	for (AActor* Actor : OverlappingActors)
	{
		ACharacter* Player = Cast<ACharacter>(Actor);
		if (Player && Player != Slime)
		{
			Slime->ApplyKnockback(Player);
			Slime->GetCharacterMovement()->MaxWalkSpeed = Slime->GetPatrolSpeed();
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}
	}
	
	if (ChargedDistance >= MaxChargeDistance)
	{
		Slime->GetCharacterMovement()->MaxWalkSpeed = Slime->GetPatrolSpeed();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
