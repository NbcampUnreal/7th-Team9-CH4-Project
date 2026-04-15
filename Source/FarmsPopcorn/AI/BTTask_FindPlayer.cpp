#include "AI/BTTask_FindPlayer.h"

#include "AIController.h"
#include "FPSlimeCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UBTTask_FindPlayer::UBTTask_FindPlayer()
{
	NodeName = TEXT("FindPlayer");
}

EBTNodeResult::Type UBTTask_FindPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AFPSlimeCharacter* Slime = Cast<AFPSlimeCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Slime) return EBTNodeResult::Failed;
	
	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(Slime, ACharacter::StaticClass(), Players);
	
	for (AActor* Actor : Players)
	{
		ACharacter* Player = Cast<ACharacter>(Actor);
		if (!Player || Player == Slime) continue;
		
		float Distance = FVector::Dist(Slime->GetActorLocation(), Player->GetActorLocation());
		
		if (Distance <= Slime->GetDetectionRadius())
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsObject(PlayerKey.SelectedKeyName, Player);
			return EBTNodeResult::Succeeded;
		}
	}
	OwnerComp.GetBlackboardComponent()->SetValueAsObject(PlayerKey.SelectedKeyName, nullptr);
	return EBTNodeResult::Failed;
}
