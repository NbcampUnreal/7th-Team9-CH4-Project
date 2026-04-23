#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BlinkRed.generated.h"

UCLASS()
class FARMSPOPCORN_API UBTTask_BlinkRed : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_BlinkRed();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:
	FTimerHandle BlinkTimerHandle;
	FTimerHandle EndTimerHandle;
};
