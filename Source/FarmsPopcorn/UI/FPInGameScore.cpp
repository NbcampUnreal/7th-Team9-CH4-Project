#include "UI/FPInGameScore.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Game/FPGameState.h"

auto UFPInGameScore::NativeTick(const FGeometry& MyGeometry, float InDeltaTime) -> void
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	TryBindCountdownEvent();

	if (AFPGameState* GS = GetWorld()->GetGameState<AFPGameState>())
	{
		if (Txt_BlueScore)
		{
			Txt_BlueScore->SetText(FText::AsNumber(GS->BlueTeamScore));
		}
		if (Txt_RedScore)
		{
			Txt_RedScore->SetText(FText::AsNumber(GS->RedTeamScore));
		}
	}
}


void UFPInGameScore::NativeConstruct()
{
	Super::NativeConstruct();
    
	CountdownAlpha = 0.0f;
	CountdownScale = 0.0f;

	// 카운트다운 패널 초기화 (숨김)
	if (CountdownPanel)
	{
		CountdownPanel->SetVisibility(ESlateVisibility::Hidden);
	}
    
	TryBindCountdownEvent();
}

void UFPInGameScore::NativeDestruct()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(CountdownHideTimerHandle);
	}

	if (bCountdownBound)
	{
		if (AFPGameState* GS = GetWorld()->GetGameState<AFPGameState>())
		{
			GS->OnCountdownTick.RemoveDynamic(this, &UFPInGameScore::OnCountdownTickReceived);
		}
		bCountdownBound = false;
	}

	Super::NativeDestruct();
}

void UFPInGameScore::OnCountdownTickReceived(int32 RemainingSeconds)
{
	if (!CountdownPanel || !Txt_Countdown)
	{
		return;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(CountdownHideTimerHandle);
	}

	// 카운트다운 패널 표시
	CountdownPanel->SetVisibility(ESlateVisibility::Visible);

	if (RemainingSeconds > 0)
	{
		Txt_Countdown->SetText(FText::AsNumber(RemainingSeconds));
	}
	else
	{
		Txt_Countdown->SetText(FText::FromString(TEXT("START!")));
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(
				CountdownHideTimerHandle,
				this,
				&UFPInGameScore::HideCountdownPanel,
				0.45f,
				false
			);
		}
	}

	FLinearColor TextColor = FLinearColor::White;
	TextColor.A = 1.0f;
	Txt_Countdown->SetColorAndOpacity(TextColor);
	Txt_Countdown->SetRenderScale(FVector2D(1.0f, 1.0f));

	if (CountdownBackground)
	{
		FLinearColor BgColor = FLinearColor::Black;
		BgColor.A = 0.7f;
		CountdownBackground->SetColorAndOpacity(BgColor);
	}
}

void UFPInGameScore::HideCountdownPanel()
{
	if (CountdownPanel)
	{
		CountdownPanel->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UFPInGameScore::TryBindCountdownEvent()
{
	if (bCountdownBound || !GetWorld())
	{
		return;
	}

	if (AFPGameState* GS = GetWorld()->GetGameState<AFPGameState>())
	{
		GS->OnCountdownTick.AddDynamic(this, &UFPInGameScore::OnCountdownTickReceived);
		bCountdownBound = true;
	}
}
