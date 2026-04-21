#include "UI/FPPlayerListEntryWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Player/FPPlayerController.h"

void UFPPlayerListEntryWidget::SetPlayerInfo(const FString& InName, bool bIsReady, UTexture2D* IconTexture)
{
	//닉네임 설정
	if (NicknameText)
		NicknameText->SetText(FText::FromString(InName));
	//초기 준비상태 설정
	UpdateReadyState(bIsReady);
	//아이콘 설정
	if (MySlotIcon && IconTexture)
	{
		MySlotIcon->SetBrushFromTexture(IconTexture);
	}
}

void UFPPlayerListEntryWidget::UpdateReadyState(bool bIsReady)
{
	//준비상태에 따라 텍스트 변경
	if (ReadyStateText)
		ReadyStateText->SetText(
			FText::FromString(bIsReady ? TEXT("준비완료") : TEXT("대기중"))
		);
}
