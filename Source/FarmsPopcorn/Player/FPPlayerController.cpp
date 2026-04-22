#include "Player/FPPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UI/FPResultWidget.h"
#include "UI/FPUIManagerSubsystem.h"
#include "UI/FPLobbyWidget.h"
#include "HAL/IConsoleManager.h"
#include "FPPlayerState.h"
#include "Game/FPGameMode.h"
#include "Engine/World.h"
#include "Game/FPGameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "Game/FPGameState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameUserSettings.h"
#include "Player/FPPlayerCharacter.h"
#include "UI/FPLoadingWidget.h"
#include "GameFramework/PlayerController.h"


void AFPPlayerController::BeginPlay()
{
	Super::BeginPlay();

	CreateNameWidgetIfNeeded();
	CreateLobbyWidgetIfNeeded();
	ForceSetScalabilityToMedium();
	if (IsLocalController())
	{
		UFPGameInstance* GI = GetGameInstance<UFPGameInstance>();
		if (GI && !GI->SaveCharacterID.IsNone() && GI->SaveCharacterClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("서버에 캐릭터 복구 요청 중: %s"), *GI->SaveCharacterID.ToString());
			Server_RestoreCharacter(GI->SaveCharacterID, GI->SaveCharacterClass);
		}
		FString MapName = GetWorld()->GetMapName();
		bool bIsMenuMap = MapName.Contains(TEXT("Login")) || MapName.Contains(TEXT("Lobby")) || MapName.Contains(TEXT("Create")) || MapName.Contains(TEXT("Result"));
		if (MapName.Contains(TEXT("Lobby")))
		{
			ServerAssignRandomCharacterIndex();
		}
		if (!bIsMenuMap)
		{
			UE_LOG(LogTemp, Warning, TEXT("인게임 맵 진입 확인 (%s): UI 복구를 시도합니다."), *MapName);
			CheckGameStateAndCreateUI();
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("메뉴 맵 진입 (%s): 점수판을 생성하지 않습니다."), *MapName);
		}
	}
	
}

void AFPPlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();

	if (IsLocalController())
	{
		FString MapName = GetWorld()->GetMapName();
		bool bIsMenuMap =
			MapName.Contains(TEXT("Login")) ||
			MapName.Contains(TEXT("Lobby")) ||
			MapName.Contains(TEXT("Create")) ||
			MapName.Contains(TEXT("Result"));

		if (bIsMenuMap)
		{
			bShowMouseCursor = true;
			return;
		}
	}

	ApplyGameplayInputMode();

	if (IsLocalController())
	{
		FString MapName = GetWorld()->GetMapName();
		bool bIsMenuMap =
			MapName.Contains(TEXT("Login")) ||
			MapName.Contains(TEXT("Lobby")) ||
			MapName.Contains(TEXT("Create")) ||
			MapName.Contains(TEXT("Result"));

		if (!bIsMenuMap)
		{
			CheckGameStateAndCreateUI();
		}
	}
}

void AFPPlayerController::ServerRequestCharacterUpdate_Implementation(int32 NewIndex)
{
	AFPGameState* GS = GetWorld() ? GetWorld()->GetGameState<AFPGameState>() : nullptr;
	AFPPlayerState* PS = GetPlayerState<AFPPlayerState>();
	AFPGameMode* GM = GetWorld() ? Cast<AFPGameMode>(GetWorld()->GetAuthGameMode()) : nullptr;

	if (!GS || !PS || !GM)
	{
		return;
	}

	const int32 MaxCharacterCount = GM->GetCharacterCount();
	if (NewIndex < 0 || NewIndex >= MaxCharacterCount)
	{
		return;
	}

	if (MyCurrentOccupiedIndex == NewIndex)
	{
		ClientSyncCharacterSelection(NewIndex);
		return;
	}

	if (GS->OccupiedIndices.Contains(NewIndex))
	{
		ClientSyncCharacterSelection(MyCurrentOccupiedIndex >= 0 ? MyCurrentOccupiedIndex : PS->CharacterIndex);
		return;
	}

	GS->UpdateOccupiedIndex(MyCurrentOccupiedIndex, NewIndex);

	MyCurrentOccupiedIndex = NewIndex;
	PS->CharacterIndex = NewIndex;

	GM->ApplyCharacterSelectionFromIndex(PS, NewIndex);
	ClientSyncCharacterSelection(NewIndex);

	UE_LOG(LogTemp, Warning, TEXT("최종 캐릭터 인덱스 적용: %s -> %d"),
		*PS->GetPlayerName(), NewIndex);
}
void AFPPlayerController::PostSeamlessTravel()
{

	Super::PostSeamlessTravel();

	if (IsLocalController())
	{
		FTimerHandle UITimer;
		GetWorldTimerManager().SetTimer(UITimer, [this]() {
			FString MapName = GetWorld()->GetMapName();
			bool bIsMenuMap = MapName.Contains(TEXT("Login")) || MapName.Contains(TEXT("Lobby")) || MapName.Contains(TEXT("Create"));
			if (!bIsMenuMap)
			{
				CheckGameStateAndCreateUI();
			}
			}, 0.1f, false);
	}
	
}
void AFPPlayerController::ForceSetScalabilityToMedium()
{
	// 콘솔 매니저에서 해당 변수를 직접 찾습니다.
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ViewDistanceQuality"));
    
	if (CVar)
	{
		// 핵심: 1로 설정하면서 권한을 ECVF_SetByCode로 줍니다.
		// 이렇게 하면 스샷에 보신 'LastSetBy: Scalability'가 'LastSetBy: Code'로 바뀌며 
		// 엔진이 함부로 3으로 되돌리지 못합니다.
		CVar->Set(1, ECVF_SetByCode);
	}

	// 다른 주요 항목들도 똑같이 처리해줍니다.
	TArray<FString> ScalabilityVars = {
		TEXT("sg.ShadowQuality"),
		TEXT("sg.AntiAliasingQuality"),
		TEXT("sg.PostProcessQuality"),
		TEXT("sg.TextureQuality"),
		TEXT("sg.EffectsQuality")
	};

	for (const FString& VarName : ScalabilityVars)
	{
		if (IConsoleVariable* TargetCVar = IConsoleManager::Get().FindConsoleVariable(*VarName))
		{
			TargetCVar->Set(1, ECVF_SetByCode);
		}
	}
}

void AFPPlayerController::ApplyGameplayInputMode()
{
	if (!IsLocalController())
	{
		return;
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

void AFPPlayerController::ServerSetCustomName_Implementation(const FString& NewName)
{
	// UI에서 받은 이름을 플레이어 스테이트에게 전달
	if (NewName.IsEmpty())return;
	if (AFPPlayerState* PS = GetPlayerState<AFPPlayerState>())
	{
		PS->CustomPlayerName = NewName;
	}

}

void AFPPlayerController::CheckGameStateAndCreateUI()
{
	//UI매니저 가져오기
	UFPUIManagerSubsystem* UISubsystem = GetGameInstance()->GetSubsystem<UFPUIManagerSubsystem>();
	if (!UISubsystem)
	{
		return;
	}
	//위젯 중복 확인
	if (UISubsystem->PersistentScoreWidget)
	{
		InGameScoreWidget = UISubsystem->PersistentScoreWidget;

		if (!InGameScoreWidget->IsInViewport())
		{
			InGameScoreWidget->AddToViewport();

			InGameScoreWidget->SetVisibility(ESlateVisibility::Visible);
			
		}
		return;
	}
	if (GetWorld() && GetWorld()->GetGameState<AFPGameState>())
	{
		if (InGameScoreWidgetClass)
		{
			InGameScoreWidget = CreateWidget<UUserWidget>(this, InGameScoreWidgetClass);
			if (InGameScoreWidget)
			{
				InGameScoreWidget->AddToViewport();
				InGameScoreWidget->SetVisibility(ESlateVisibility::Visible);
				UISubsystem->PersistentScoreWidget = InGameScoreWidget;

			}
			else
			{
				FTimerHandle TimerHandle;
				GetWorldTimerManager().SetTimer(TimerHandle, this, &AFPPlayerController::CheckGameStateAndCreateUI, 0.2f, false);
			}
		}
	}
}

void AFPPlayerController::SetReady(bool bNewReadyState)
{
	if (AFPPlayerState* PS = GetPlayerState<AFPPlayerState>())
	{
		if (PS->bIsReady == bNewReadyState) return;
		PS->Server_SetReady(bNewReadyState);
	}
	if (AFPGameMode* GM = Cast<AFPGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->CheckAllPlayersReady();
	}
}
void AFPPlayerController::ClientHideRoundResult_Implementation()
{
	if (RoundResultWidget && RoundResultWidget->IsInViewport())
	{
		RoundResultWidget->RemoveFromViewport();//뷰포트 제거
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
	}
}
void AFPPlayerController::ClientShowRoundResult_Implementation(int32 InRedTeamScore, int32 InBlueTeamScore)
{
	if (!RoundResultWidgetClass)
	{
		return;
	}

	if (!RoundResultWidget)
	{
		RoundResultWidget = CreateWidget<UUserWidget>(this, RoundResultWidgetClass);
	}

	if (!RoundResultWidget)
	{
		return;
	}

	if (!RoundResultWidget->IsInViewport())
	{
		RoundResultWidget->AddToViewport(10);
	}

	if (InGameScoreWidget)
	{
		InGameScoreWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	if (UFPResultWidget* RoundUI = Cast<UFPResultWidget>(RoundResultWidget))
	{
		RoundUI->RedTeamScore = InRedTeamScore;
		RoundUI->BlueTeamScore = InBlueTeamScore;
		RoundUI->OnResultUpdated();
	}

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(RoundResultWidget->TakeWidget());
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AFPPlayerController::Server_RestoreCharacter_Implementation(FName SavedID, TSubclassOf<APawn> SavedClass)
{
	if (!HasAuthority()) return;

	AFPPlayerState* PS = GetPlayerState<AFPPlayerState>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_RestoreCharacter: PlayerState NULL"));
		return;
	}
	if (UFPGameInstance* GI = GetGameInstance<UFPGameInstance>())
	{
		GI->SaveCharacterID = SavedID;
		GI->SaveCharacterClass = SavedClass;
		UE_LOG(LogTemp, Warning, TEXT("GameInstance 동기화: %s"), *SavedID.ToString());
	}
	if (PS)
	{
		PS->AssignedCharacterID = SavedID;
		PS->AssignedCharacterClass = SavedClass;

		if (AFPGameMode* GM = GetWorld()->GetAuthGameMode<AFPGameMode>())
		{
			GM->RestartPlayer(this);
		}
	}
}



void AFPPlayerController::Client_SaveCharacterToInstance_Implementation(FName CharID, TSubclassOf<APawn> CharClass)
{
	UFPGameInstance* GI = GetGameInstance<UFPGameInstance>();
	if (GI)
	{
		GI->SaveCharacterID = CharID;
		GI->SaveCharacterClass = CharClass;

		UE_LOG(LogTemp, Warning, TEXT("내 캐릭터 정보 (%s)가 인스턴스에 저장되었습니다."), *CharID.ToString());
	}
}

void AFPPlayerController::ServerSendChatMessage_Implementation(const FString& SenderName, const FString& Message)
{
	//보낸 사람의 ID 가져오기
	AFPPlayerState* PS = GetPlayerState<AFPPlayerState>();
	EFPTeamID SenderTeam = PS ? PS->TeamID : EFPTeamID::None;
	//빈 문자열 무시
	if (SenderName.IsEmpty() || Message.IsEmpty()) return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AFPPlayerController* TargetPC = Cast<AFPPlayerController>(It->Get()))
		{
			TargetPC->ClientReceiveChatMessage(SenderName, Message, SenderTeam);
		}
	}
}
void AFPPlayerController::ClientReceiveChatMessage_Implementation(const FString& SenderName, const FString& Message, EFPTeamID TeamID)
{
	if (OnChatMessageReceived.IsBound())
	{
		OnChatMessageReceived.Broadcast(SenderName, Message, TeamID);
	}
	else
	{
		PendingMessages.Add(FPendingChatMessage(SenderName, Message, TeamID));
	}
}
void AFPPlayerController::ClientShowFinalResult_Implementation()
{

}

void AFPPlayerController::ClientShowPostTravelLoading_Implementation(TSubclassOf<UFPLoadingWidget> LoadingClass, float Duration, const FString& LoadingText)
{
	if (!IsLocalController() || !LoadingClass)
	{
		return;
	}

	UFPLoadingWidget* LoadingWidget = CreateWidget<UFPLoadingWidget>(this, LoadingClass);
	if (!LoadingWidget)
	{
		return;
	}

	LoadingWidget->AddToViewport(200);
	LoadingWidget->StartLoading(ELoadingType::Custom, Duration);
	if (!LoadingText.IsEmpty())
	{
		LoadingWidget->SetLoadingText(LoadingText);
	}
}

void AFPPlayerController::ClientSetCountdownInputLock_Implementation(bool bLocked)
{
	SetIgnoreMoveInput(bLocked);
	SetIgnoreLookInput(bLocked);
	FlushPressedKeys();

	if (AFPPlayerCharacter* PC = Cast<AFPPlayerCharacter>(GetPawn()))
	{
		PC->GetCharacterMovement()->StopMovementImmediately();
		PC->ConsumeMovementInputVector();
	}
}

void AFPPlayerController::ClientSyncCharacterSelection_Implementation(int32 ConfirmedIndex)
{
	MyCurrentOccupiedIndex = ConfirmedIndex;
}

void AFPPlayerController::DebugEndRound()
{
	// 서버인지 확인
	if (HasAuthority())
	{
		if (AFPGameMode* GM = Cast<AFPGameMode>(GetWorld()->GetAuthGameMode()))
		{
			// 테스트용 점수 주고 라운드 종료
			GM->AddScoreToTeam(EFPTeamID::TeamRed, 10);
			GM->EndRound();

			UE_LOG(LogTemp, Warning, TEXT("디버그: 라운드 강제 종료 실행!"));
		}
	}
}

void AFPPlayerController::RequestChangeTeam()
{
	ServerRequestChangeTeam();
}
void AFPPlayerController::ServerAssignRandomCharacterIndex_Implementation()
{
	AFPGameState* GS = GetWorld() ? GetWorld()->GetGameState<AFPGameState>() : nullptr;
	AFPPlayerState* PS = GetPlayerState<AFPPlayerState>();
	AFPGameMode* GM = Cast<AFPGameMode>(GetWorld()->GetAuthGameMode());

	if (!GS || !PS || !GM)
	{
		return;
	}

	if (MyCurrentOccupiedIndex != -1)
	{
		return;
	}

	const int32 MaxCharacterCount = GM->GetCharacterCount();

	TArray<int32> AvailableIndices;
	for (int32 i = 0; i < MaxCharacterCount; ++i)
	{
		if (!GS->OccupiedIndices.Contains(i))
		{
			AvailableIndices.Add(i);
		}
	}

	if (AvailableIndices.Num() <= 0)
	{
		return;
	}

	const int32 RandomArrayIndex = FMath::RandRange(0, AvailableIndices.Num() - 1);
	const int32 ChosenIndex = AvailableIndices[RandomArrayIndex];

	// 랜덤도 최종 적용은 같은 함수로 통일
	ServerRequestCharacterUpdate(ChosenIndex);
}
void AFPPlayerController::CreateLobbyWidgetIfNeeded()
{
	UE_LOG(LogTemp, Warning, TEXT("CreateLobbyWidgetIfNeeded called"));

	if (!IsLocalController())
	{
		UE_LOG(LogTemp, Warning, TEXT("Not local controller"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("World is null"));
		return;
	}

	const FString MapName = World->GetMapName();
	UE_LOG(LogTemp, Warning, TEXT("Current MapName: %s"), *MapName);

	const bool bIsLobbyMap = MapName.Contains(TEXT("Lobby"));
	if (!bIsLobbyMap)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not lobby map"));
		return;
	}

	if (LobbyWidgetInstance && LobbyWidgetInstance->IsInViewport())
	{
		UE_LOG(LogTemp, Warning, TEXT("Lobby widget already exists"));
		return;
	}

	if (!LobbyWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("LobbyWidgetClass is null"));
		return;
	}

	LobbyWidgetInstance = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
	if (!LobbyWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateWidget failed"));
		return;
	}

	LobbyWidgetInstance->AddToViewport();
	UE_LOG(LogTemp, Warning, TEXT("Lobby widget added to viewport"));

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(LobbyWidgetInstance->TakeWidget());
	SetInputMode(InputMode);
	bShowMouseCursor = true;

	UE_LOG(LogTemp, Warning, TEXT("로비 UI 입력 모드 설정 완료, 마우스 커서 표시"));
}
void AFPPlayerController::CreateNameWidgetIfNeeded()
{
	if (!IsLocalController())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FString MapName = World->GetMapName();
	const bool bIsCreateMap = MapName.Contains(TEXT("Create"));

	if (!bIsCreateMap)
	{
		return;
	}

	if (CreateNameWidgetInstance && CreateNameWidgetInstance->IsInViewport())
	{
		return;
	}

	if (!CreateNameWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateNameWidgetClass is null"));
		return;
	}

	CreateNameWidgetInstance = CreateWidget<UUserWidget>(this, CreateNameWidgetClass);
	if (!CreateNameWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to create CreateNameWidget"));
		return;
	}

	CreateNameWidgetInstance->AddToViewport();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(CreateNameWidgetInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}
bool AFPPlayerController::ServerRequestChangeTeam_Validate() { return true; }

void AFPPlayerController::ServerRequestChangeTeam_Implementation()
{
	if (AFPGameMode* GM = Cast<AFPGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->ChangeTeam(this);
	}
}
