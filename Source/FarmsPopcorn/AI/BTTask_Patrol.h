#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Patrol.generated.h"

UCLASS()
class FARMSPOPCORN_API UBTTask_Patrol : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_Patrol();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
protected:
	//순찰 반경
	UPROPERTY(EditAnywhere, Category = "Patrol Setting")
	float PatrolRadius = 2000.f;
};