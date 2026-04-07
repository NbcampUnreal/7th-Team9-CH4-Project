#include "UI/FPLobbyWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UFPLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	//UI 전용 마우스 커서 활성화
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputMode);
	}
	//초기 텍스트설정
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(TEXT("대기중...")));
	}
	if (ReadyButtonText)
	{
		ReadyButtonText->SetText(FText::FromString(TEXT("준비")));
	}
	//준비 버튼 누를시 활성화
	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddDynamic(this, &UFPLobbyWidget::OnReadyClicked);
	}
	
}

void UFPLobbyWidget::OnReadyClicked()
{
	bIsReady = !bIsReady; //준비상태 취소상태
	//UI 갱신
	if (bIsReady)
	{
		if (StatusText) StatusText->SetText(FText::FromString(TEXT("준비 완료")));
		if (ReadyButtonText) ReadyButtonText->SetText(FText::FromString(TEXT("취소")));
	}
	else
	{
		if (StatusText) StatusText->SetText(FText::FromString(TEXT("대기 중...")));
		if (ReadyButtonText) ReadyButtonText->SetText(FText::FromString(TEXT("준비")));
	}
}

void UFPLobbyWidget::UpdateReadyStatus(bool bNewReadyState)
{
	//텍스트 업데이트
	if (bNewReadyState)
	{
		StatusText->SetText(FText::FromString(TEXT("준비 완료")));
		ReadyButtonText->SetText(FText::FromString(TEXT("취소")));
	}
	else
	{
		StatusText->SetText(FText::FromString(TEXT("대기 중...")));
		ReadyButtonText->SetText(FText::FromString(TEXT("준비")));
	}
}
