#include "Game/FPGameMode.h"
#include "FPGameState.h"
#include "FPGameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameUserSettings.h"
#include "Player/FPPlayerCharacter.h"
#include "Player/FPPlayerState.h"
#include "Player/FPPlayerController.h"
#include "UI/FPLoadingWidget.h"
#include "UI/FPCharacterDefinition.h"

namespace
{
	FString GetRoundMapPath(const int32 RoundNumber)
	{
		switch (RoundNumber)
		{
		case 1: return TEXT("/Game/Maps/Game/L_Game_Round01");
		case 2: return TEXT("/Game/Maps/Game/L_Game_Round02");
		case 3: return TEXT("/Game/Maps/Game/L_Game_Round03");
		default: return FString();
		}
	}

	FString GetFinalResultMapPath()
	{
		return TEXT("/Game/Maps/L_ScoreResultWidgetTest");
	}
}


AFPGameMode::AFPGameMode()
{
	bUseSeamlessTravel = true;
}

void AFPGameMode::PostLogin(APlayerController* NewPlayer)
{
	if (AvatarClasses.IsEmpty() && CharacterDataTable)
	{
		CheckPlayerAvater();
	}

	AssignTeam(NewPlayer);
	Super::PostLogin(NewPlayer);
}
void AFPGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	UFPGameInstance* GI = GetGameInstance<UFPGameInstance>();
	if (GI)
	{
		RedTeamScore = GI->SaveRedScore;
		BlueTeamScore = GI->SaveBlueScore;
		
		// ✅ GI에서 CurrentRound 복원
		CurrentRound = GI->SaveCurrentRound;
		if (CurrentRound == 0) CurrentRound = 1;  // 최초 시작
		
		UE_LOG(LogTemp, Warning, 
			TEXT("BeginPlay - Round: %d, Red: %d, Blue: %d"), 
			CurrentRound, RedTeamScore, BlueTeamScore);

		if (AFPGameState* GS = GetGameState<AFPGameState>())
		{
			GS->RedTeamScore = RedTeamScore;
			GS->BlueTeamScore = BlueTeamScore;
			GS->CurrentRound = CurrentRound;  
		}
	}
}

void AFPGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);
	
	if (!C)
	{
		return;
	}
	
	APlayerController* PC = Cast<APlayerController>(C);
	if (!PC)
	{
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("✓ HandleSeamlessTravelPlayer: %s (LocalPlayer: %s)"), 
		*PC->GetName(),
		PC->IsLocalPlayerController() ? TEXT("TRUE") : TEXT("FALSE"));
	
	// SeamlessTravel로 이동한 플레이어에 대해 초기화
	if (AvatarClasses.IsEmpty() && CharacterDataTable)
	{
		CheckPlayerAvater();
	}
	AssignTeam(PC);

	if (AFPPlayerState* FPPS = PC->GetPlayerState<AFPPlayerState>())
	{
		ApplyCharacterSelectionFromIndex(FPPS, FPPS->CharacterIndex);
	}

	if (LoadingWidgetClass)
	{
		if (AFPPlayerController* FPPC = Cast<AFPPlayerController>(PC))
		{
			FPPC->ClientShowPostTravelLoading(
				LoadingWidgetClass,
				0.5f,
				TEXT("맵 로딩 중...")
			);
		}
	}
	const FString MapName = GetWorld()->GetMapName();
	if (MapName.Contains(TEXT("L_Game_Round")))
	{
		if (AFPPlayerCharacter* FPPC = Cast<AFPPlayerCharacter>(PC->GetPawn()))
		{
			FPPC->GetCharacterMovement()->SetMovementMode(MOVE_None);
		}
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
	UE_LOG(LogTemp, Warning, TEXT("✓ Player initialized after seamless travel"));
}

void AFPGameMode::Logout(AController* Exiting)
{
	if (AFPPlayerController* FPC = Cast<AFPPlayerController>(Exiting))
	{
		AFPGameState* GS = GetGameState<AFPGameState>();
		AFPPlayerState* PS = FPC->GetPlayerState<AFPPlayerState>();

		if (GS && PS)
		{
			GS->OccupiedIndices.Remove(PS->CharacterIndex);
			FPC->MyCurrentOccupiedIndex = -1;
			PS->CharacterIndex = -1;
		}
	}

	Super::Logout(Exiting);
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
	if (FPPlayerState->AssignedCharacterClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("플레이어 %s는 이미 캐릭터가 할당되어 있습니다: %s"),
			*FPPlayerState->GetPlayerName(),
			*FPPlayerState->AssignedCharacterID.ToString());
		return;
	}
	// ===== 아바타 검증 =====
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
 
			// UFPGameInstance* GI = GetGameInstance<UFPGameInstance>();
			// if (GI)
			// {
			// 	GI->SaveCharacterID = RowPair.Key;
			// 	GI->SaveCharacterClass = SelectedClass;
			// 	GI->SaveCharacterName = RowData->CharacterName;
			// 	GI->SaveCharacterIcon = RowData->CharacterIcon;
			// }
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

void AFPGameMode::ApplyCharacterSelectionFromIndex(AFPPlayerState* FPPlayerState, int32 CharacterIndex)
{
	if (!FPPlayerState || !CharacterDefinitions.IsValidIndex(CharacterIndex))
	{
		return;
	}

	UFPCharacterDefinition* Definition = CharacterDefinitions[CharacterIndex];
	if (!Definition || !Definition->CharacterClass)
	{
		return;
	}

	FPPlayerState->AssignedCharacterID = Definition->CharacterID;
	FPPlayerState->AssignedCharacterClass = Definition->CharacterClass;
	FPPlayerState->AssignedCharacterName = Definition->CharacterName.ToString();
	FPPlayerState->AssignedCharacterIcon = Definition->CharacterIcon;
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
	UFPGameInstance* GI = GetGameInstance<UFPGameInstance>();
	
	if (FPPlayerState->TeamID == EFPTeamID::TeamRed)
	{
		if (BlueTeamCount >= MaxPlayersPerTeam)
		{
			UE_LOG(LogTemp,Log,TEXT("플레이어가 블루팀인원이 가득차서 팀을 변경할수 없습니다."));
			return;
		}
		FPPlayerState->TeamID =EFPTeamID::TeamBlue;
		GI->SaveTeamID = EFPTeamID::TeamBlue;
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
		GI->SaveTeamID = EFPTeamID::TeamRed;
		RedTeamCount++;  
		BlueTeamCount--; 
	}
	UpdateTeamCounts();
}

// 준비 상태 체크
void AFPGameMode::CheckAllPlayersReady()
{
	UpdateTeamCounts();
    
	if (RedTeamCount != BlueTeamCount) 
	{
		UE_LOG(LogTemp, Warning, TEXT("팀 인원이 같지 않음! 리턴"));
		return;
	}
    
	// 복제 동기화를 위해 약간의 딜레이 추가
	if (ReadyCheckHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(ReadyCheckHandle);
	}
    
	GetWorld()->GetTimerManager().SetTimer(
		ReadyCheckHandle,
		this,
		&AFPGameMode::DelayedReadyPlayerCheck,
		0.1f,  // 100ms 딜레이
		false
	);
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

	const FString MapName = GetWorld()->GetMapName();
	if (MapName.Contains(TEXT("L_Game_Round")))
	{
		FTimerDelegate StartRoundDelegate = FTimerDelegate::CreateLambda([this]()
		{
			if (AFPGameState* CurrentGS = GetGameState<AFPGameState>())
			{
				CurrentGS->SetGamePhase(EFPGamePhase::InGame);
			}
		});

		GetWorld()->GetTimerManager().SetTimer(
			CountDownHandle,
			StartRoundDelegate,
			3.0f,//수정
			false
		);
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		CountDownHandle,
		this,
		&AFPGameMode::ExecuteMapTravel, //카운트 다운 UI사라짐, 캐릭터 퍼즈
		3.0f,
		false
	);
}

bool AFPGameMode::ReadyPlayerCheck()
{
	if (!GameState || GameState->PlayerArray.Num() == 0)
		return false;
    
	int32 ReadyCount = 0;
	int32 TotalPlayers = GameState->PlayerArray.Num();
    
	for (APlayerState* PS : GameState->PlayerArray)
	{
		AFPPlayerState* FPPS = Cast<AFPPlayerState>(PS);
		if (FPPS)
		{
			if (FPPS->bIsReady)
			{
				ReadyCount++;
			}
			else
			{
				// 디버깅: 준비 안 된 플레이어 로그
				UE_LOG(LogTemp, Warning, 
					TEXT("플레이어 %s는 아직 준비 안 함"), 
					*FPPS->GetPlayerName());
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("준비 상태: %d / %d"), ReadyCount, TotalPlayers);
    
	// 최소 2명 이상 필요 
	return ReadyCount == TotalPlayers && TotalPlayers >= 2;
}
void AFPGameMode::DelayedReadyPlayerCheck()
{
	if (!GameState || GameState->PlayerArray.Num() == 0)
		return;
    
	int32 ReadyCount = 0;
	int32 TotalPlayers = GameState->PlayerArray.Num();
    
	UE_LOG(LogTemp, Warning, TEXT("=== 준비 상태 최종 체크 시작 ==="));
    
	for (APlayerState* PS : GameState->PlayerArray)
	{
		AFPPlayerState* FPPS = Cast<AFPPlayerState>(PS);
		if (FPPS)
		{
			UE_LOG(LogTemp, Warning, 
				TEXT("플레이어 %s - bIsReady: %s"), 
				*FPPS->GetPlayerName(), 
				FPPS->bIsReady ? TEXT("TRUE") : TEXT("FALSE"));
            
			if (FPPS->bIsReady)
			{
				ReadyCount++;
			}
		}
	}
    
	UE_LOG(LogTemp, Warning, TEXT("준비 상태: %d / %d"), ReadyCount, TotalPlayers);
    
	// 모든 조건을 명시적으로 확인
	bool bAllReady = (ReadyCount == TotalPlayers) && (TotalPlayers >= 2);
    
	if (bAllReady)
	{
		// 추가 안전 장치: 실제로 모두 준비 상태인지 다시 한 번 확인
		bool bDoubleCheck = true;
		for (APlayerState* PS : GameState->PlayerArray)
		{
			AFPPlayerState* FPPS = Cast<AFPPlayerState>(PS);
			if (FPPS && !FPPS->bIsReady)
			{
				bDoubleCheck = false;
				UE_LOG(LogTemp, Error,
					TEXT("이중 체크 실패! 플레이어 %s가 준비 안 함"),
					*FPPS->GetPlayerName());
				break;
			}
		}

		if (bDoubleCheck)
		{
			ExecuteMapTravel();
		}
	}
}
void AFPGameMode::ExecuteMapTravel()
{
	if (NextLevel.IsNull()) // 할당 여부 체크
	{
		UE_LOG(LogTemp, Error, TEXT("ExecuteMapTravel: NextLevel이 블루프린트에서 설정되지 않았습니다!"));
		return;
	}

	// 소프트 포인터에서 전체 경로 문자열 추출 (가장 확실한 방법)
	FString LevelPath = NextLevel.ToSoftObjectPath().GetLongPackageName();
    
	if (LevelPath.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("ExecuteMapTravel: 유효한 레벨 경로를 찾을 수 없습니다!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("서버 트래블 시도 중: %s"), *LevelPath);
	
	GetWorld()->ServerTravel(LevelPath, true);
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

int32 AFPGameMode::GetCharacterCount() const
{
	return CharacterDefinitions.Num();
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
	
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void AFPGameMode::AddScoreToTeam(EFPTeamID InTeamID, int32 ScoreAmount)
{
	if (InTeamID == EFPTeamID::TeamRed)
	{
		RedTeamScore += ScoreAmount;
	}
	else if (InTeamID == EFPTeamID::TeamBlue)
	{
		BlueTeamScore += ScoreAmount;
	}

	//UI에서 읽는 것에 즉시 반영
	if (AFPGameState* GS = GetGameState < AFPGameState>())
	{
		GS->RedTeamScore = RedTeamScore;
		GS->BlueTeamScore = BlueTeamScore;
		GS->RedTeamScore, GS->BlueTeamScore;
	}
	UFPGameInstance* GI = GetGameInstance<UFPGameInstance>();
	if (!GI)
	{
		return;
	}
	// 3. GI가 유효할 때만 점수를 누적 기록합니다. (안전장치)
	if (InTeamID == EFPTeamID::TeamRed)
	{
		GI->SaveRedScore = RedTeamScore;
	}
	else if (InTeamID == EFPTeamID::TeamBlue)
	{
		GI->SaveBlueScore = BlueTeamScore;
	}

}

void AFPGameMode::EndRound()
{
	//점수 동기화
	AFPGameState* GS = GetGameState<AFPGameState>();
	if (GS)
	{
		GS->RedTeamScore = RedTeamScore; 
		GS->BlueTeamScore = BlueTeamScore;
		
		GS->SetGamePhase(EFPGamePhase::Result);

		if (RedTeamScore > BlueTeamScore)
		{
			GS->RoundWinnerTeam = EFPTeamID::TeamRed;
		}
		else if (BlueTeamScore > RedTeamScore)
		{
			GS->RoundWinnerTeam = EFPTeamID::TeamBlue;
		}
	}
	
	// ✅ 결과 UI 표시
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AFPPlayerController* PC = Cast<AFPPlayerController>(It->Get());
		if (PC)
		{
			PC->ClientShowRoundResult(RedTeamScore, BlueTeamScore);
		}
	}
	GetWorldTimerManager().SetTimer(
		RoundResultTimerHandle,
		this,
		&AFPGameMode::StartNextRound,
		ResultDisplayTime,  // 1초
		false
	);
	
	UE_LOG(LogTemp, Warning, TEXT("=== EndRound - 결과 화면 표시 (%.1f초 후 다음 라운드) ==="), ResultDisplayTime);
}

void AFPGameMode::StartNextRound()
{
	AFPGameState* GS = GetGameState<AFPGameState>();
	if (!GS) return;
	
	GS->RedTotalScore += GS->RedTeamScore;
	GS->BlueTotalScore += GS->BlueTeamScore;

	if (CurrentRound >= MaxRounds)
	{
		UE_LOG(LogTemp, Warning, TEXT("=== 게임 종료 (CurrentRound: %d, MaxRounds: %d) ==="), CurrentRound, MaxRounds);
		
		if (UFPGameInstance* GI = GetGameInstance<UFPGameInstance>())
		{
			GI->SaveCurrentRound = 0;
			GI->SaveRedScore = 0;
			GI->SaveBlueScore = 0;
		}

		if (LoadingWidgetClass)
		{
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				if (AFPPlayerController* PC = Cast<AFPPlayerController>(It->Get()))
				{
					PC->ClientShowPostTravelLoading(LoadingWidgetClass, 2.0f, TEXT("최종 결과 화면으로 이동 중..."));
				}
			}
		}

		const FString FinalMapPath = GetFinalResultMapPath();
		FTimerDelegate FinalTravelDelegate = FTimerDelegate::CreateLambda([this, FinalMapPath]()
		{
			GetWorld()->ServerTravel(FinalMapPath + TEXT("?listen"), true);
		});
		GetWorldTimerManager().SetTimer(RoundResultTimerHandle, FinalTravelDelegate, 2.0f, false);

		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			AFPPlayerController* PC = Cast<AFPPlayerController>(It->Get());
			if (PC) PC->ClientShowFinalResult();
		}
		return;
	}
	
	CurrentRound++;
	GS->CurrentRound = CurrentRound;
	
	if (UFPGameInstance* GI = GetGameInstance<UFPGameInstance>())
	{
		GI->SaveCurrentRound = CurrentRound;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Round 진행: %d / %d"), CurrentRound, MaxRounds);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AFPPlayerController* PC = Cast<AFPPlayerController>(It->Get());
		if (PC) PC->ClientHideRoundResult();
	}

	RedTeamScore = 0;
	BlueTeamScore = 0;
	GS->RedTeamScore = 0;
	GS->BlueTeamScore = 0;

	const FString NextRoundPath = GetRoundMapPath(CurrentRound);
	if (NextRoundPath.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT(" 다음 라운드 맵 경로를 찾을 수 없습니다. Round: %d"), CurrentRound);
		return;
	}

	if (LoadingWidgetClass)
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (AFPPlayerController* PC = Cast<AFPPlayerController>(It->Get()))
			{
				PC->ClientShowPostTravelLoading(
					LoadingWidgetClass,
					1.0f,  
					FString::Printf(TEXT("Round %d 준비 중..."), CurrentRound)
				);
			}
		}
	}

	GetWorld()->ServerTravel(NextRoundPath + TEXT("?listen"), true);
	
	UE_LOG(LogTemp, Warning, TEXT("=== Round %d 이동 중 ==="), CurrentRound);
}

void AFPGameMode::ShowRoundTransitionLoading()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->IsLocalPlayerController() && LoadingWidgetClass)
		{
			CurrentLoadingWidget = CreateWidget<UFPLoadingWidget>(PC, LoadingWidgetClass);
			if (CurrentLoadingWidget)
			{
				// GameMode의 레벨 경로를 Widget에 주입
				if (!RoundNextLevel.IsNull())
				{
					CurrentLoadingWidget->OverrideLevelPath = 
						RoundNextLevel.ToSoftObjectPath().GetLongPackageName();
				}

				CurrentLoadingWidget->StartLoading(ELoadingType::RoundTransition, 1.0f);
				CurrentLoadingWidget->SetLoadingText(FString::Printf(
					TEXT("Round %d\n\n레드팀: %d\n블루팀: %d"),
					CurrentRound, RedTeamScore, BlueTeamScore
				));
				CurrentLoadingWidget->AddToViewport(100);
				CurrentLoadingWidget->OnLoadingComplete.AddDynamic(
					this, &AFPGameMode::OnRoundTransitionLoadingComplete);

				UE_LOG(LogTemp, Warning, TEXT("✓ Round Transition Loading shown - Round %d"), CurrentRound);
				break;
			}
		}
	}
}
void AFPGameMode::OnRoundTransitionLoadingComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("✓ Round Transition Loading Complete - Starting next round"));
	
	// 다음 라운드 준비
	CurrentRound++;
	
	AFPGameState* GS = GetGameState<AFPGameState>();
	if (GS)
	{
		GS->CurrentRound = CurrentRound;
		GS->SetGamePhase(EFPGamePhase::InGame);
	}
	
	// 라운드 점수만 초기화 (누적은 유지)
	RedTeamScore = 0;
	BlueTeamScore = 0;
	
	if (GS)
	{
		GS->RedTeamScore = 0;
		GS->BlueTeamScore = 0;
	}
	
	// 모든 플레이어에게 라운드 결과 UI 숨김
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AFPPlayerController* PC = Cast<AFPPlayerController>(It->Get());
		if (PC)
		{
			PC->ClientHideRoundResult();
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("=== Round %d 시작 ==="), CurrentRound);
}

