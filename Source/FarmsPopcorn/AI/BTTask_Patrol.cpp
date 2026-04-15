#include "BTTask_Patrol.h"

#include "AIController.h"
#include "FPSlimeCharacter.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_Patrol::UBTTask_Patrol()
{
	NodeName = TEXT("Patrol");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_Patrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AFPSlimeCharacter* Slime = Cast<AFPSlimeCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Slime) return EBTNodeResult::Failed;
	
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(Slime->GetWorld());
	if (!NavSystem) return EBTNodeResult::Failed;
	
	FNavLocation RandomLocation;
	bool bFound = NavSystem->GetRandomReachablePointInRadius(Slime->GetActorLocation(), PatrolRadius, RandomLocation);
	if (!bFound) return EBTNodeResult::Failed;
	
	AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
	AIController->MoveToLocation(RandomLocation.Location, 20.0f, true,true, false, true);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_Patrol::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	if (AIController->GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}