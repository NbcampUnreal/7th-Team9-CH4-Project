#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "FPSlimeAIController.generated.h"

UCLASS()
class FARMSPOPCORN_API AFPSlimeAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AFPSlimeAIController();
	
	virtual void OnPossess(APawn* InPawn) override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<class UBehaviorTreeComponent> BTComponent;
	
	UPROPERTY(EditAnywhere, Category = "AI")
	class UBlackboardComponent* BBComponent;
	
	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<class UBehaviorTree> BehaviorTree;

	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<class UBlackboardData> BlackboardData;
};
