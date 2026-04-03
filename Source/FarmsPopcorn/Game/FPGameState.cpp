#include "Game/FPGameState.h"

#include "Net/UnrealNetwork.h"


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

void AFPGameState::OnRep_CustomPlayerName()
{
}

void AFPGameState::OnRep_IsReady()
{
}

void AFPGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPGameState, RemainingTime);
	DOREPLIFETIME(AFPGameState, CustomPlayerName);
	DOREPLIFETIME(AFPGameState, bAllReady);
	DOREPLIFETIME(AFPGameState, GamePhase);
	DOREPLIFETIME(AFPGameState, RedTeamScore);
	DOREPLIFETIME(AFPGameState, BlueTeamScore);
}
