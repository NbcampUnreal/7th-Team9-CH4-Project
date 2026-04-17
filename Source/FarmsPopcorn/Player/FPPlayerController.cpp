#include "Player/FPPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UI/FPResultWidget.h"
#include "FPPlayerState.h"
#include "Game/FPGameMode.h"
#include "Engine/World.h"
#include "Game/FPGameInstance.h"
#include "GameFramework/PlayerController.h"


void AFPPlayerController::BeginPlay()
{
	Super::BeginPlay();
	ApplyGameplayInputMode();
	
	if (IsLocalController())
	{
		UFPGameInstance* GI = GetGameInstance<UFPGameInstance>();
		if (GI && !GI->SaveCharacterID.IsNone() && GI->SaveCharacterClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("서버에 캐릭터 복구 요청 중: %s"), *GI->SaveCharacterID.ToString());
			Server_RestoreCharacter(GI->SaveCharacterID, GI->SaveCharacterClass);
		}
	}
}

void AFPPlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();
	ApplyGameplayInputMode();
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
		*PS->GetPlayerName());
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
	//빈 문자열 무시
	if (SenderName.IsEmpty() || Message.IsEmpty()) return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AFPPlayerController* OtherPC = Cast<AFPPlayerController>(It->Get());
		if (OtherPC)
		{
			// 서버가 각 클라이언트의 'ClientReceiveChatMessage'를 실행시킴
			OtherPC->ClientReceiveChatMessage(SenderName, Message);
		}
	}
}
void AFPPlayerController::ClientReceiveChatMessage_Implementation(const FString& SenderName, const FString& Message)
{
	if (OnChatMessageReceived.IsBound())
	{
		OnChatMessageReceived.Broadcast(SenderName, Message);
	}
}
void AFPPlayerController::ClientShowFinalResult_Implementation()
{

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
