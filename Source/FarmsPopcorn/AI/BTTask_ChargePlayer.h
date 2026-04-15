#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ChargePlayer.generated.h"

UCLASS()
class FARMSPOPCORN_API UBTTask_ChargePlayer : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_ChargePlayer();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PlayerKey;
	
	UPROPERTY(EditAnywhere, Category = "Charge Setting")
	float MaxChargeDistance = 10000.f;
	
	FVector ChargedDirection;
	float ChargedDistance = 0.f;
};
