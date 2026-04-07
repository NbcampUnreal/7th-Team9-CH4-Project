#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPLobbyWidget.generated.h"

UCLASS()
class FARMSPOPCORN_API UFPLobbyWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:
	virtual void NativeConstruct() override;
protected:
	//닉네임 표시
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayNickName;
	//레디 상태 표사
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* StatusText;
	//레디 버튼
	UPROPERTY(meta = (BindWidget))
	class UButton* ReadyButton;
	//레디 버튼 택스트
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ReadyButtonText;

	UFUNCTION()
	void OnReadyClicked();

	void UpdateReadyStatus(bool bNewReadyState);

private:
	bool bIsReady = false; //현재 레디 상태
};
