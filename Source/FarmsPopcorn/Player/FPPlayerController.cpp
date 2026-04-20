#include "Player/FPPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UI/FPResultWidget.h"
#include "UI/FPUIManagerSubsystem.h"
#include "FPPlayerState.h"
#include "Game/FPGameMode.h"
#include "Engine/World.h"
#include "Game/FPGameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "Game/FPGameState.h"
#include "UI/FPLoadingWidget.h"
#include "GameFramework/PlayerController.h"


void AFPPlayerController::BeginPlay()
{
	Super::BeginPlay();

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
		UE_LOG(LogTemp, Warning, TEXT("플레이어 %s의 "),
			*PS->GetPlayerName(), *NewName);
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
void AFPPlayerController::ClientShowRoundResult_Implementation()
{
	if (!RoundResultWidgetClass) return;
	//위젯 생성
	if (!RoundResultWidget)
	{
		RoundResultWidget = CreateWidget<UUserWidget>(this, RoundResultWidgetClass);
	}
	if (RoundResultWidget && !RoundResultWidget->IsInViewport())
	{
		RoundResultWidget->AddToViewport(10);

		if (InGameScoreWidget)
		{
			InGameScoreWidget->SetVisibility(ESlateVisibility::Hidden);
		}


		UFPResultWidget* RoundUI = Cast<UFPResultWidget>(RoundResultWidget);
		if (RoundUI)
		{
			RoundUI->UpdateRoundResult();
		}
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(RoundResultWidget->TakeWidget());
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
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
bool AFPPlayerController::ServerRequestChangeTeam_Validate() { return true; }

void AFPPlayerController::ServerRequestChangeTeam_Implementation()
{
	if (AFPGameMode* GM = Cast<AFPGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->ChangeTeam(this);
	}
}
