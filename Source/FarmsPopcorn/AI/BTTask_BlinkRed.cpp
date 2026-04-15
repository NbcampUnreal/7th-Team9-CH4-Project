#include "BTTask_BlinkRed.h"

#include "AIController.h"
#include "AI/FPSlimeCharacter.h"

UBTTask_BlinkRed::UBTTask_BlinkRed()
{
	NodeName = TEXT("Task_BlinkRed");
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_BlinkRed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AFPSlimeCharacter* Slime = Cast<AFPSlimeCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Slime) return EBTNodeResult::Failed;
	
	//깜박임 시작
	bool bBlink = true;
	FTimerDelegate BlinkDelegate;
	BlinkDelegate.BindLambda([Slime, &bBlink]() mutable
	{
		bBlink = !bBlink;
		Slime->SetBlinkRed(bBlink);
	});
	
	//0.2초마다 깜박임
	Slime->GetWorldTimerManager().SetTimer(BlinkTimerHandle, BlinkDelegate, 0.5f, true);
	
	//깜박임 종료
	UBehaviorTreeComponent* BTComp = &OwnerComp;
	FTimerDelegate EndDelegate;
	EndDelegate.BindLambda([Slime, BTComp, this]()
	{
		Slime->GetWorldTimerManager().ClearTimer(BlinkTimerHandle);
		Slime->SetBlinkRed(false);
		FinishLatentTask(*BTComp, EBTNodeResult::Succeeded);
	});
	
	Slime->GetWorldTimerManager().SetTimer(EndTimerHandle, EndDelegate, 1.0f, false);
	
	return EBTNodeResult::InProgress;
}
