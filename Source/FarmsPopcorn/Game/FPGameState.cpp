#include "Game/FPGameState.h"


AFPGameState::AFPGameState()
{
}

void AFPGameState::OnRep_GamePhase()
{
	switch (GamePhase)
	{
	case EFPGamePhase::CountDown:
		
		break;
            
	case EFPGamePhase::InGame:
		
		break;
            
	case EFPGamePhase::Result:
		
		break;
	}
}
