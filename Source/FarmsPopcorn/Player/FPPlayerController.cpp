#include "Player/FPPlayerController.h"
#include "FPPlayerState.h"
#include "Game/FPGameMode.h"
#include "Engine/World.h"
#include "Game/FPGameInstance.h"
#include "GameFramework/PlayerController.h"


void AFPPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
	
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

void AFPPlayerController::Server_RestoreCharacter_Implementation(FName SavedID, TSubclassOf<APawn> SavedClass)
{
	if (AFPPlayerState* PS = GetPlayerState<AFPPlayerState>())
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