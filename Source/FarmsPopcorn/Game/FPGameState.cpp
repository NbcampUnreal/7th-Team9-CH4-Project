
#include "Game/FPGameState.h"
#include "Game/FPGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/FPPlayerCharacter.h"
#include "Player/FPPlayerState.h"


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
		const FString MapName = GetWorld()->GetMapName();
		if (MapName.Contains(TEXT("L_Game_Round")))
		{
			EarliestCountdownStartTime = GetWorld()->GetTimeSeconds() + 2.1f;
			// 모든 플레이어의 로딩/폰 소유가 끝난 뒤 카운트다운 시작
			GetWorld()->GetTimerManager().SetTimer(
				RoundStartCheckHandle,
				this,
				&AFPGameState::TryStartRoundCountdown,
				0.2f,
				true,
				0.1f
			);
			return;
		}

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
	if (!HasAuthority())
	{
		return;
	}

	SetPlayersCountdownLocked(true);
	RemainingTime = 3.0f;
	OnRep_RemainingTime();
	GetWorld()->GetTimerManager().ClearTimer(CountDownTickHandle);
	GetWorld()->GetTimerManager().SetTimer(
		CountDownTickHandle,
		this,
		&AFPGameState::TickCountDown,
		1.0f,
		true
	);
}

void AFPGameState::OnRep_RemainingTime()
{
	int32 Sec = FMath::CeilToInt(RemainingTime); // 2.9 → 3, 1.0 → 1
	OnCountdownTick.Broadcast(FMath::Max(0, Sec));
}
void AFPGameState::HandleGameStarted()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(CountDownTickHandle);
	RemainingTime = 0.0f;
	OnRep_RemainingTime();
	SetPlayersCountdownLocked(false);
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

void AFPGameState::TickCountDown()
{
	if (!HasAuthority())
	{
		return;
	}

	RemainingTime = FMath::Max(0.0f, RemainingTime - 1.0f);
	OnRep_RemainingTime();
	if (RemainingTime <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(CountDownTickHandle);
	}
}

void AFPGameState::TryStartRoundCountdown()
{
	if (!HasAuthority())
	{
		return;
	}

	if (GetWorld()->GetTimeSeconds() < EarliestCountdownStartTime)
	{
		return;
	}

	int32 HumanPlayers = 0;
	int32 ReadyPlayers = 0;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->PlayerState || PC->PlayerState->IsABot())
		{
			continue;
		}

		HumanPlayers++;
		if (PC->GetPawn())
		{
			ReadyPlayers++;
		}
	}

	if (HumanPlayers > 0 && HumanPlayers == ReadyPlayers)
	{
		GetWorld()->GetTimerManager().ClearTimer(RoundStartCheckHandle);
		if (AFPGameMode* FPGameMode = Cast<AFPGameMode>(GetWorld()->GetAuthGameMode()))
		{
			FPGameMode->StartGameCountdown();
		}
	}
}

void AFPGameState::SetPlayersCountdownLocked(bool bLocked)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AFPPlayerController* FPPC = Cast<AFPPlayerController>(It->Get()))
		{
			FPPC->ClientSetCountdownInputLock(bLocked);
		}

		if (AFPPlayerCharacter* Character = Cast<AFPPlayerCharacter>(It->Get() ? It->Get()->GetPawn() : nullptr))
		{
			Character->GetCharacterMovement()->StopMovementImmediately();
			Character->ConsumeMovementInputVector();
			if (bLocked)
			{
				Character->GetCharacterMovement()->SetMovementMode(MOVE_None);
			}
			else
			{
				Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			}
		}
	}
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
	DOREPLIFETIME(AFPGameState, OccupiedIndices);
}

