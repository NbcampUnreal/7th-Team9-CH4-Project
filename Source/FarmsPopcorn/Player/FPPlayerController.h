#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPPlayerController.generated.h"

//채팅 저장
USTRUCT(BlueprintType)
struct FPendingChatMessage
{
	GENERATED_BODY()

	UPROPERTY()
	FString SenderName;

	UPROPERTY()
	FString Message;

	FPendingChatMessage()
	{
	}
	FPendingChatMessage(const FString& InSenderName, const FString& InMessage) : SenderName(InSenderName), Message(InMessage)
	{
	}
};
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnChatMessageReceived, const FString&, const FString&);

UCLASS()
class FARMSPOPCORN_API AFPPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Name")
	FString InName;
public:
	UFUNCTION(Server, Reliable)
	void ServerSetCustomName(const FString& NewName);
	
	void SetReady(bool bNewReadyState);
	
	FOnChatMessageReceived OnChatMessageReceived;

	UFUNCTION(Server, Reliable)
	void ServerSendChatMessage(const FString& SenderName, const FString& Message);

	UFUNCTION(Client, Reliable)
	void ClientReceiveChatMessage(const FString& SenderName, const FString& Message);

	//위젯이 읽어갈 채팅
	UPROPERTY()
	TArray<FPendingChatMessage> PendingMessages;
};
