
#include "Game/FPGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


AFPGameState::AFPGameState()
{
	bAllReady = false;
	GamePhase = EFPGamePhase::Lobby;
}



void AFPGameState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
		if (GameMode && UGameplayStatics::HasOption(GameMode->OptionsString, TEXT("Phase")))
		{
			FString PhaseStr = UGameplayStatics::ParseOption(GameMode->OptionsString, TEXT("Phase"));
            
			if (PhaseStr == TEXT("InGame"))
			{
				SetGamePhase(EFPGamePhase::InGame);
			}
		}
	}
}
void AFPGameState::OnRep_GamePhase()
{
	switch (GamePhase)
	{
	case EFPGamePhase::CountDown:
		HandleCountDownStarted();
		break;
            
	case EFPGamePhase::InGame:
		HandleGameStarted();
		break;
            
	case EFPGamePhase::Result:
		HandleResultStarted();
		break;
	}
}

void AFPGameState::SetGamePhase(EFPGamePhase NewPhase)
{
	if (HasAuthority()) 
	{
		GamePhase = NewPhase; 
		OnRep_GamePhase();  
	}
}


void AFPGameState::HandleCountDownStarted()
{
}

void AFPGameState::HandleGameStarted()
{
}

void AFPGameState::HandleResultStarted()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	AFPPlayerController* FPPC = Cast <AFPPlayerController>(PC);
	if (FPPC)
	{
		FPPC->ClientShowRoundResult();
	}
}


void AFPGameState::OnRep_IsReady()
{
	
}



void AFPGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPGameState, RemainingTime);
	DOREPLIFETIME(AFPGameState, bAllReady);
	DOREPLIFETIME(AFPGameState, GamePhase);
	DOREPLIFETIME(AFPGameState, RedTeamScore);
	DOREPLIFETIME(AFPGameState, BlueTeamScore);
	DOREPLIFETIME(AFPGameState, RedTotalScore);
	DOREPLIFETIME(AFPGameState, BlueTotalScore);
	DOREPLIFETIME(AFPGameState, CurrentRound);
}
