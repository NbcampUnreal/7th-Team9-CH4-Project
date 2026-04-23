#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPInGameScore.generated.h"

class UTextBlock;
class UImage;
class UCanvasPanel;
UCLASS()
class FARMSPOPCORN_API UFPInGameScore : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	void TryBindCountdownEvent();
	
protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_BlueScore;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_RedScore;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_Countdown;
	UPROPERTY(meta = (BindWidget))
	UImage* CountdownBackground;
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* CountdownPanel; 
	
	float CountdownAlpha;
	float CountdownScale;
	bool bCountdownBound = false;
	FTimerHandle CountdownHideTimerHandle;
	
	FDelegateHandle CountdownDelegateHandle;
	
	float LastRemainingTime = -1.0f;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime)override;
	void HideCountdownPanel();
	UFUNCTION()
	void OnCountdownTickReceived(int32 RemainingSeconds);
};
