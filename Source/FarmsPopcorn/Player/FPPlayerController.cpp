#include "Player/FPPlayerController.h"
#include "FPPlayerState.h"
#include "Game/FPGameMode.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void AFPPlayerController::ServerSetCustomName_Implementation(const FString& NewName)
{
	// UI에서 받은 이름을 플레이어 스테이트에게 전달
	if (NewName.IsEmpty())return;
	if (AFPPlayerState* PS = GetPlayerState<AFPPlayerState>())
	{
		PS->CustomPlayerName = NewName;
	}
}

void AFPPlayerController::Server_SetReady_Implementation(bool bNewReadyState)
{
	AFPPlayerState* PS = GetPlayerState<AFPPlayerState>();
	
	UE_LOG(LogTemp, Warning, TEXT("플레이어 %s의 준비 상태: %s"), 
		*PS->GetPlayerName(), bNewReadyState ? TEXT("준비") : TEXT("대기"));
    
	PS->bIsReady = bNewReadyState;
	AFPGameMode* GM = Cast<AFPGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->CheckAllPlayersReady(); 
	}
}
void AFPPlayerController::ServerSendChatMessage_Implementation(const FString& SenderName, const FString& Message)
{
	// 빈 문자열이면 전송하지 않음
	if (SenderName.IsEmpty() || Message.IsEmpty())
	{
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		AFPPlayerController* OtherPC = Cast<AFPPlayerController>(It->Get());
		if (!OtherPC)
		{
			continue;
		}
		OtherPC->PendingMessages.Add(FPendingChatMessage(SenderName, Message));
	}
}