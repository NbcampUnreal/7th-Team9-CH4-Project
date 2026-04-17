#include "UI/FPInGameScore.h"
#include "Components/TextBlock.h"
#include "Game/FPGameState.h"

void UFPInGameScore::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

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
