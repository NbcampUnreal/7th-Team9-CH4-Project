#include "Game/FPGameMode.h"
#include "FPGameState.h"
#include "Player/FPPlayerState.h"

AFPGameMode::AFPGameMode()
{
}

void AFPGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	AssignTeam(NewPlayer);
}


//팀인원 파악
void AFPGameMode::UpdateTeamCounts()
{
	RedTeamCount = 0;
	BlueTeamCount = 0;
	
	if (GameState)
	{
		for (APlayerState* PS : GameState->PlayerArray)
		{
			AFPPlayerState* FPPlayerState = Cast<AFPPlayerState>(PS);
			if (FPPlayerState)
			{
				if (FPPlayerState->TeamID == EFPTeamID::TeamRed)
				{
					RedTeamCount++;
				}else if (FPPlayerState->TeamID == EFPTeamID::TeamBlue)
				{
					BlueTeamCount++;
				}
			}
		}
	}
	UE_LOG(LogTemp, Log, TEXT("현재 팀 인원 갱신 완료 - Red: %d / Blue: %d"), RedTeamCount, BlueTeamCount);
}

// 팀자동 이동
void AFPGameMode::AssignTeam(AController* Player)
{
	if (!Player || !Player->PlayerState) return;
    
	AFPPlayerState* FPPlayerState = Cast<AFPPlayerState>(Player->PlayerState);
	if (!FPPlayerState) return;
	//플레이어가 이미 팀이 배정되어 있다면(None이 아니라면) 무시
	if (FPPlayerState->TeamID != EFPTeamID::None) return;
	
	UpdateTeamCounts();
	
	// 둘 다 0명이거나, 블루팀이 레드팀보다 많거나 같을 경우 -> 레드팀
	if (RedTeamCount <= BlueTeamCount) 
	{
		FPPlayerState->TeamID = EFPTeamID::TeamRed;
		UE_LOG(LogTemp, Log, TEXT("플레이어 %s가 레드팀에 배정되었습니다."), *FPPlayerState->GetPlayerName());
	}
	// 레드팀이 블루팀보다 많을 경우 -> 블루팀
	else 
	{
		FPPlayerState->TeamID = EFPTeamID::TeamBlue;
		UE_LOG(LogTemp, Log, TEXT("플레이어 %s가 블루팀에 배정되었습니다."), *FPPlayerState->GetPlayerName());
	}
	UpdateTeamCounts();
}
// 팀변경
void AFPGameMode::ChangeTeam(AController* Player)
{
	if (!Player || !Player->PlayerState) return;
	AFPPlayerState* FPPlayerState = Cast<AFPPlayerState>(Player->PlayerState);
	if (!FPPlayerState) return;
	UpdateTeamCounts();
	
	if (FPPlayerState->TeamID == EFPTeamID::TeamRed)
	{
		if (BlueTeamCount >= MaxPlayersPerTeam)
		{
			UE_LOG(LogTemp,Log,TEXT("플레이어가 블루팀인원이 가득차서 팀을 변경할수 없습니다."))
			return;
		}
		FPPlayerState->TeamID =EFPTeamID::TeamBlue;
	}else if(FPPlayerState->TeamID == EFPTeamID::TeamBlue)
	{
		if (RedTeamCount >= MaxPlayersPerTeam)
		{
			UE_LOG(LogTemp,Log,TEXT("플레이어가 레드팀인원이 가득차서 팀을 변경할수 없습니다."))
			return;
		}
		FPPlayerState->TeamID =EFPTeamID::TeamRed;
	}
	UpdateTeamCounts();
}

void AFPGameMode::CheckAllPlayersReady()
{
	// 아직 한 명도 안 들어왔다면 무시
	if (GameState->PlayerArray.IsEmpty()) return;

	bool bAllReady = true;
	ReadyPlayerCheck(bAllReady);
	// 전원 준비 완료
	if (bAllReady)
	{
		UE_LOG(LogTemp, Warning, TEXT("모든 플레이어가 준비되었습니다! 카운트다운을 시작합니다."));
		StartGameCountdown();
	}
}

void AFPGameMode::StartGameCountdown()
{
	AFPGameState* GS = GetGameState<AFPGameState>();
	if (GS)
	{
		GS->SetGamePhase(EFPGamePhase::CountDown);
	}
	
	if (CountDownHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(CountDownHandle);
	}
	GetWorld()->GetTimerManager().SetTimer(
		CountDownHandle,
		this,
		&AFPGameMode::ExecuteMapTravel,
		3.0f,
		false
		);
}

void AFPGameMode::ReadyPlayerCheck(bool Ready)
{
	// 플레이어 레디 상태 확인
	if (GameState)
	{
		for (APlayerState* PS : GameState->PlayerArray)
		{
			AFPPlayerState* FPPS = Cast<AFPPlayerState>(PS);
			if (FPPS && !FPPS->bIsReady)
			{
				Ready = false;
				break;
			}
		}
	}
}

void AFPGameMode::ExecuteMapTravel()
{
	UWorld* World = GetWorld();
	// 게임 
	if (World)
	{
		GetWorld()->ServerTravel("/Game/Maps/InGameMap?listen?Phase=InGame");
	}
}


void AFPGameMode::SpawnAndPossessRandomAvatar(AController* Player)
{
}


