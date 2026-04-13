#pragma once

#include "CoreMinimal.h"
#include "Player/FPPlayerController.h"
#include "FPLobbyPlayerController.generated.h"

UCLASS()
class FARMSPOPCORN_API AFPLobbyPlayerController : public AFPPlayerController
{
	GENERATED_BODY()
public:
    // 채팅 메시지 서버로 전송
    UFUNCTION(Server, Reliable)
    void ServerSendChatMessage(const FString& SenderName, const FString& Message);

    // 모든 클라이언트에 채팅 메시지 전달
    UFUNCTION(NetMulticast, Reliable)
    void MulticastReceiveChatMessage(const FString& SenderName, const FString& Message);

    // 수신된 메시지 목록 
    TArray<TPair<FString, FString>> PendingMessages;
};
