#include "UI/FPResultWidget.h"
#include "Game/FPGameState.h"

void UFPResultWidget::UpdateRoundResult()
{
	AFPGameState* GS = GetWorld()->GetGameState<AFPGameState>();
	if (!GS)
	{
		return;
	}

	RedTeamScore = GS->RedTeamScore;
	BlueTeamScore = GS->BlueTeamScore;

	OnResultUpdated();
}
