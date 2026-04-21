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
    
	// Slime과 BehaviorTreeComponent가 타이머 대기 중 파괴될 수 있으므로 Weak Pointer로 감싼다?
	TWeakObjectPtr<AFPSlimeCharacter> WeakSlime = Slime;
	TWeakObjectPtr<UBehaviorTreeComponent> WeakBTComp = &OwnerComp;
    
	// 깜박임 시작
	bool bBlink = true;
	FTimerDelegate BlinkDelegate;

	// &bBlink 참조 대신 bBlink 값 캡처, WeakSlime을 사용 isvalid로 처리
	BlinkDelegate.BindLambda([WeakSlime, bBlink]() mutable
	{
		if (WeakSlime.IsValid()) // 슬라임이 살아있을 때만 실행
		{
			bBlink = !bBlink;
			WeakSlime->SetBlinkRed(bBlink);
		}
	});
    
	// 0.5초마다 깜박임
	Slime->GetWorldTimerManager().SetTimer(BlinkTimerHandle, BlinkDelegate, 0.5f, true);
    
	// 깜빡임 종료
	FTimerDelegate EndDelegate;

	// 생포인터 BTComp 대신 WeakBTComp 사용, WeakSlime 사용
	EndDelegate.BindLambda([WeakSlime, WeakBTComp, this]()
	{
		// 슬라임이 살아있다면 타이머 정리, 색상 원상복구
		if (WeakSlime.IsValid())
		{
			WeakSlime->GetWorldTimerManager().ClearTimer(BlinkTimerHandle);
			WeakSlime->SetBlinkRed(false);
		}
        
		// BT가 아직 유효하다면 Task 완료 처리
		if (WeakBTComp.IsValid())
		{
			// WeakBTComp.Get()으로 실제 포인터를 가져와 역참조(*~)하여 전달
			FinishLatentTask(*WeakBTComp.Get(), EBTNodeResult::Succeeded);
		}
	});
    
	Slime->GetWorldTimerManager().SetTimer(EndTimerHandle, EndDelegate, 1.0f, false);
    
	return EBTNodeResult::InProgress;
}
