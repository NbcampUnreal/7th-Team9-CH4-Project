#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPResultWidget.generated.h"

UCLASS()
class FARMSPOPCORN_API UFPResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	//라운드 종료시 점수와 승리 정보 UI
	UFUNCTION(BlueprintCallable,Category = "Result")
	void UpdateRoundResult();
	//블루프린트에서 디자인한 텍스트나 애니메이션을 제어하기 위한 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Result")
	void OnResultUpdated();

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	int32 RedTeamScore = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	int32 BlueTeamScore = 0;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UFPResultWidget> RoundResultWidgetClass;
};
