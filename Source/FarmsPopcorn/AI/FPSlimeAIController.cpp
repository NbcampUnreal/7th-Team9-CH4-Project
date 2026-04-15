#include "AI/FPSlimeAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"

AFPSlimeAIController::AFPSlimeAIController()
{
	BTComponent = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTreeComponent");
	BBComponent = CreateDefaultSubobject<UBlackboardComponent>("BlackboardComponent");
}

void AFPSlimeAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (BehaviorTree && BlackboardData)
	{
		UseBlackboard(BlackboardData,BBComponent);
		RunBehaviorTree(BehaviorTree);
	}
}
