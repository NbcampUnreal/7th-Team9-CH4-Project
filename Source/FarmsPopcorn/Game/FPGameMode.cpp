#include "Game/FPGameMode.h"
#include "FPGameState.h"
#include "Player/FPPlayerState.h"


void AFPGameMode::PostLogin(APlayerController* NewPlayer)
{
	if (AvatarClasses.IsEmpty() && CharacterDataTable)
	{
		CheckPlayerAvater();
	}
	AssignCharacterToPlayer(NewPlayer);
	AssignTeam(NewPlayer);
	Super::PostLogin(NewPlayer);
}

//아바타 데이터테이블 정보확인
void AFPGameMode::AssignCharacterToPlayer(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("AssignCharacterToPlayer: PlayerController가 NULL"));
		return;
	}
 
	APlayerState* PS = PlayerController->PlayerState;
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("AssignCharacterToPlayer: PlayerState가 NULL"));
		return;
	}
 
	AFPPlayerState* FPPlayerState = Cast<AFPPlayerState>(PS);
	if (!FPPlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("AssignCharacterToPlayer: FPPlayerState 캐스트 실패"));
		return;
	}
 
	// ===== 아바타 검증 =====
	if (AvatarClasses.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("AssignCharacterToPlayer: 사용 가능한 아바타가 없음"));
		return;
	}
 
	if (AvatarClasses.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("AssignCharacterToPlayer: AvatarClasses 배열이 비어있음"));
		return;
	}
 
	// ===== 데이터테이블 검증 =====
	if (!CharacterDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("AssignCharacterToPlayer: CharacterDataTable이 NULL"));
		return;
	}
 
	// 무작위 선택
	int32 RandomIndex = FMath::RandRange(0, AvatarClasses.Num() - 1);
	TSubclassOf<APawn> SelectedClass = AvatarClasses[RandomIndex];
 
	if (!SelectedClass)
	{
		UE_LOG(LogTemp, Error, TEXT("AssignCharacterToPlayer: SelectedClass가 NULL"));
		return;
	}
 
	// ===== 데이터테이블에서 정보 찾기 =====
	bool bFound = false;
	for (auto& RowPair : CharacterDataTable->GetRowMap())
	{
		//Safe cast
		auto RowData = reinterpret_cast<FFPCharacterDataRow*>(RowPair.Value);
		
		if (!RowData)
		{
			continue;
		}
 
		// 캐릭터 에셋 비교
		TSubclassOf<APawn> RowClass = RowData->CharacterAsset.Get();
		if (RowClass == SelectedClass)
		{
			// ===== PlayerState에 정보 저장 =====
			FPPlayerState->AssignedCharacterID = RowPair.Key;
			FPPlayerState->AssignedCharacterClass = SelectedClass;
			FPPlayerState->AssignedCharacterName = RowData->CharacterName;
			FPPlayerState->AssignedCharacterIcon = RowData->CharacterIcon;
 
			UsedCharacterIDs.Add(RowPair.Key);
 
			UE_LOG(LogTemp, Warning,
				TEXT(" 플레이어 '%s'에게 할당: %s (이름: %s, 남은 아바타: %d)"),
				*PlayerController->GetName(),
				*RowPair.Key.ToString(),
				*RowData->CharacterName,
				AvatarClasses.Num() - 1);
 
			bFound = true;
			break;
		}
	}
 
	if (!bFound)
	{
		UE_LOG(LogTemp, Warning, TEXT("AssignCharacterToPlayer: 데이터테이블에서 캐릭터를 찾지 못함"));
	}
 
	// 중복 방지
	if (AvatarClasses.IsValidIndex(RandomIndex))
	{
		AvatarClasses.RemoveAt(RandomIndex);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AssignCharacterToPlayer: 유효하지 않은 인덱스 %d (배열 크기: %d)"),
			RandomIndex, AvatarClasses.Num());
	}
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

// 준비 상태 체크
void AFPGameMode::CheckAllPlayersReady()
{
	// 블루팀과 레드팀의 수가 같지 않으면 리턴
	UpdateTeamCounts();
	UE_LOG(LogTemp, Error, TEXT("Red: %d / Blue: %d"), RedTeamCount, BlueTeamCount);
    
	if (RedTeamCount != BlueTeamCount) 
	{
		UE_LOG(LogTemp, Warning, TEXT("팀 인원이 같지 않음! 리턴"));
		return;
	}
    
	if (ReadyPlayerCheck())
	{
		UE_LOG(LogTemp, Warning, TEXT("모든 플레이어가 준비되었습니다! 카운트다운을 시작합니다."));
		StartGameCountdown();
	}
	else
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
	UsedCharacterIDs.Reset();  
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
	if (!InController || !InController->PlayerState)
	{
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}
 
	AFPPlayerState* FPPlayerState = Cast<AFPPlayerState>(InController->PlayerState);
	
	if (FPPlayerState && FPPlayerState->AssignedCharacterClass)
	{
		UE_LOG(LogTemp, Log, TEXT("플레이어 %s의 할당된 캐릭터 사용: %s"),
			*InController->GetName(),
			*FPPlayerState->AssignedCharacterID.ToString());
		
		return FPPlayerState->AssignedCharacterClass;
	}
	UE_LOG(LogTemp, Error, TEXT("플레이어 %s에게 할당된 캐릭터가 없습니다!"), *InController->GetName());
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}