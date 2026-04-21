#include "UI/FPLobbyCenterSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

void UFPLobbyCenterSlotWidget::SetPlayerInfo(const FString& InName, bool bIsReady, UTexture2D* IconTexture, EFPTeamID TeamID)
{
	SetVisibility(ESlateVisibility::Visible);

	if (Img_Character)
	{
		Img_Character->SetBrushFromTexture(IconTexture);
	}

	if (Txt_PlayerName)
	{
		Txt_PlayerName->SetText(FText::FromString(InName));
	}

	if (Txt_ReadyState)
	{
		Txt_ReadyState->SetText(FText::FromString(bIsReady ? TEXT("준비 완료") : TEXT("대기 중")));
		Txt_ReadyState->SetColorAndOpacity(
			bIsReady ? FSlateColor(FLinearColor::Green) : FSlateColor(FLinearColor::White)
		);
	}
	if (Txt_TeamLabel)
	{
		if (TeamID == EFPTeamID::TeamBlue)
		{
			Txt_TeamLabel->SetText(FText::FromString(TEXT("블루팀")));
			Txt_TeamLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.2f, 0.5f, 1.0f, 1.0f)));
		}
		else if (TeamID == EFPTeamID::TeamRed)
		{
			Txt_TeamLabel->SetText(FText::FromString(TEXT("레드팀")));
			Txt_TeamLabel->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.3f, 0.3f, 1.0f)));
		}
	}

	if (Bg_TeamColor)
	{
		if (TeamID == EFPTeamID::TeamBlue)
		{
			Bg_TeamColor->SetBrushColor(FLinearColor(0.3f, 0.6f, 1.0f, 0.9f));
		}
		else if (TeamID == EFPTeamID::TeamRed)
		{
			Bg_TeamColor->SetBrushColor(FLinearColor(1.0f, 0.4f, 0.4f, 0.9f));
		}
		else
		{
			Bg_TeamColor->SetBrushColor(FLinearColor::White);
		}
	}
}

void UFPLobbyCenterSlotWidget::SetEmpty()
{
	SetVisibility(ESlateVisibility::Hidden);

	if (Img_Character)
	{
		Img_Character->SetBrushFromTexture(nullptr);
	}

	if (Txt_PlayerName)
	{
		Txt_PlayerName->SetText(FText::GetEmpty());
	}

	if (Txt_TeamLabel)
	{
		Txt_TeamLabel->SetText(FText::GetEmpty());
	}

	if (Txt_ReadyState)
	{
		Txt_ReadyState->SetText(FText::GetEmpty());
	}
}