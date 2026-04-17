#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPInGameScore.generated.h"

class UTextBlock;

UCLASS()
class FARMSPOPCORN_API UFPInGameScore : public UUserWidget
{
	GENERATED_BODY()
	

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_BlueScore;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_RedScore;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime)override;
};
