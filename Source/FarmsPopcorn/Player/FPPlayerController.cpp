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