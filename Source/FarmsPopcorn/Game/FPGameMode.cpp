#include "Game/FPGameMode.h"
#include "FPGameState.h"
#include "Player/FPPlayerState.h"


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
	//플레이어가 이미 팀이 배정되어 있다면(None이 아니라면) 무시
	if (!FPPlayerState || FPPlayerState->TeamID != EFPTeamID::None) return;
	
	UpdateTeamCounts();
	//모든 팀이 가득차있을시 
	if (RedTeamCount >= MaxPlayersPerTeam && BlueTeamCount >= MaxPlayersPerTeam)
	{
		UE_LOG(LogTemp, Warning, TEXT("모든 팀이 가득 찼습니다!"));
		return;
	}
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
			UE_LOG(LogTemp,Log,TEXT("플레이어가 블루팀인원이 가득차서 팀을 변경할수 없습니다."));
			return;
		}
		FPPlayerState->TeamID =EFPTeamID::TeamBlue;
		RedTeamCount--;  
		BlueTeamCount++;  
	}else if(FPPlayerState->TeamID == EFPTeamID::TeamBlue)
	{
		if (RedTeamCount >= MaxPlayersPerTeam)
		{
			UE_LOG(LogTemp,Log,TEXT("플레이어가 레드팀인원이 가득차서 팀을 변경할수 없습니다."));
			return;
		}
		FPPlayerState->TeamID =EFPTeamID::TeamRed;
		RedTeamCount++;  
		BlueTeamCount--; 
	}
	UpdateTeamCounts();
}

void AFPGameMode::CheckAllPlayersReady()
{
	// 블루팀과 레드팀의 수가 같지 않으면 리턴
	UpdateTeamCounts();
	if (RedTeamCount != BlueTeamCount) return;
	
	// 전원 준비 완료
	if (ReadyPlayerCheck())
	{
		UE_LOG(LogTemp, Warning, TEXT("모든 플레이어가 준비되었습니다! 카운트다운을 시작합니다."));
		StartGameCountdown();
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("모든 플레이어가 준비되지않았습니다!"));
	}
}

void AFPGameMode::StartGameCountdown()
{
	//카운트 다운 메세지 UI 3,2,1,
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

bool AFPGameMode::ReadyPlayerCheck()
{
	// 플레이어 레디 상태 확인
	if (!GameState || GameState->PlayerArray.Num() == 0)
		return false;
	for (APlayerState* PS : GameState->PlayerArray)
	{
		AFPPlayerState* FPPS = Cast<AFPPlayerState>(PS);
		if (FPPS && !FPPS->bIsReady)
		{
			return false;
		}
	}

	return true;
}

void AFPGameMode::ExecuteMapTravel()
{
	// 인게임 레벨로 이동
	UWorld* World = GetWorld();
	// 게임 
	if (World)
	{
		GetWorld()->ServerTravel("/Game/Maps/InGameMap?listen?Phase=InGame");
	}
}

void AFPGameMode::ResetAvatarSelection()
{
	AvatarClasses.Reset();
	CharacterID.Reset();
	CheckPlayerAvater();
}

void AFPGameMode::CheckPlayerAvater()
{
	if (!CharacterDataTable || !AvatarClasses.IsEmpty()) 
		return;  // 이미 로드됨
	
	for (auto& RowPair : CharacterDataTable->GetRowMap())
	{
		FFPCharacterDataRow* RowData = reinterpret_cast<FFPCharacterDataRow*>(RowPair.Value);
		if (RowData && RowData->CharacterAsset.IsValid())
		{
			UClass* LoadedClass = RowData->CharacterAsset.LoadSynchronous();
			if (LoadedClass)
			{
				AvatarClasses.Add(LoadedClass);
			}
		}
	}
}

UClass* AFPGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	CheckPlayerAvater();
    
	if (AvatarClasses.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("사용 가능한 아바타가 없습니다!"));
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}
    
	//  사용 가능한 캐릭터 중에서만 선택
	int32 RandomIndex = FMath::RandRange(0, AvatarClasses.Num() - 1);
	TSubclassOf<APawn> SelectedClass = AvatarClasses[RandomIndex];
    
	// 선택된 캐릭터를 배열에서 제거 (중복 방지)
	AvatarClasses.RemoveAt(RandomIndex);
    
	// 선택 기록 남기기
	if (CharacterDataTable)
	{
		for (auto& RowPair : CharacterDataTable->GetRowMap())
		{
			FFPCharacterDataRow* RowData = reinterpret_cast<FFPCharacterDataRow*>(RowPair.Value);
			if (RowData && RowData->CharacterAsset.Get() == SelectedClass)
			{
				CharacterID.Add(RowPair.Key);
				UE_LOG(LogTemp, Log, TEXT("캐릭터 할당: %s (남은 아바타: %d)"), 
					*RowPair.Key.ToString(), AvatarClasses.Num());
				break;
			}
		}
	}
    
	return SelectedClass;
}
