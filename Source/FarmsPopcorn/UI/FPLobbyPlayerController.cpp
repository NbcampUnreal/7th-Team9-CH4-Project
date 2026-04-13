#include "UI/FPLobbyPlayerController.h"

void AFPLobbyPlayerController::ServerSendChatMessage_Implementation(const FString& SenderName, const FString& Message)
{
    MulticastReceiveChatMessage(SenderName, Message);
}
void AFPLobbyPlayerController::MulticastReceiveChatMessage_Implementation(const FString& SenderName, const FString& Message)
{
    // 수신된 메시지 배열에 저장 → ChatWidget에서 꺼내감
    PendingMessages.Add(TPair<FString, FString>(SenderName, Message));
}