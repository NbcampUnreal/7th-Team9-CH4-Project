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

	UE_LOG(LogTemp, Warning, TEXT("BlinkRed ExecuteTask called"));

	TWeakObjectPtr<AFPSlimeCharacter> WeakSlime = Slime;
	TWeakObjectPtr<UBehaviorTreeComponent> WeakBTComp = &OwnerComp;

	bool bBlink = true;
	Slime->SetBlinkRed(bBlink);

	FTimerDelegate BlinkDelegate;
	BlinkDelegate.BindLambda([WeakSlime, bBlink]() mutable
	{
		if (!WeakSlime.IsValid()) return;
		bBlink = !bBlink;
		WeakSlime->SetBlinkRed(bBlink);
	});

	Slime->GetWorldTimerManager().SetTimer(BlinkTimerHandle, BlinkDelegate, 0.5f, true);

	FTimerDelegate EndDelegate;
	EndDelegate.BindLambda([WeakSlime, WeakBTComp, this]()
	{
		if (WeakSlime.IsValid())
		{
			WeakSlime->GetWorldTimerManager().ClearTimer(BlinkTimerHandle);
			WeakSlime->SetBlinkRed(false);
		}
		if (WeakBTComp.IsValid())
		{
			FinishLatentTask(*WeakBTComp.Get(), EBTNodeResult::Succeeded);
		}
	});

	Slime->GetWorldTimerManager().SetTimer(EndTimerHandle, EndDelegate, 1.0f, false);

	return EBTNodeResult::InProgress;
}
