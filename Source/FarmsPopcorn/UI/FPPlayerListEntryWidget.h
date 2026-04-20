#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "FPPlayerListEntryWidget.generated.h"

UCLASS()
class FARMSPOPCORN_API UFPPlayerListEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:
	//플레이어 닉네임 + 초기준비상태 설정
	UFUNCTION(BlueprintCallable)
	void SetPlayerInfo(const FString& InName, bool bIsReady, UTexture2D* IconTexture);
	//준비상태 택스트 업데이트
	UFUNCTION(BlueprintCallable)
	void UpdateReadyState(bool bIsReady);

protected:
	//닉네임 텍스트
	UPROPERTY(BlueprintReadwrite, meta = (BindWidget))
	class UTextBlock* NicknameText;
	//준비상태 텍스트
	UPROPERTY(BlueprintReadwrite, meta = (BindWidget))
	class UTextBlock* ReadyStateText;
	//플레이어 아이콘
	UPROPERTY(meta = (BindWidget))
	class UImage* PlayerIcon;
};
