#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindPlayer.generated.h"

UCLASS()
class FARMSPOPCORN_API UBTTask_FindPlayer : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_FindPlayer();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:
	//BB 키 할당
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PlayerKey;
};
