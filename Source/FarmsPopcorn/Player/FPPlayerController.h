#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Core/FPTeamID.h"
#include "FPPlayerController.generated.h"

class AFPGameState;
class UFPLoadingWidget;

//채팅 저장
USTRUCT(BlueprintType)
struct FPendingChatMessage
{
	GENERATED_BODY()

	UPROPERTY()
	FString SenderName;

	UPROPERTY()
	FString Message;

	UPROPERTY()
	EFPTeamID TeamID;

	FPendingChatMessage()
	{
	}
	FPendingChatMessage(const FString& InSenderName, const FString& InMessage, EFPTeamID InTeamID) : SenderName(InSenderName), Message(InMessage), TeamID(InTeamID)
	{
	}
};
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnChatMessageReceived, const FString&, const FString&, EFPTeamID);

UCLASS()
class FARMSPOPCORN_API AFPPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Name")
	FString InName;
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> InGameScoreWidgetClass;
	UPROPERTY()
	class UUserWidget* InGameScoreWidget;
public:
	UFUNCTION(Server, Reliable)
	void ServerSetCustomName(const FString& NewName);
	
	void SetReady(bool bNewReadyState);
	
	FOnChatMessageReceived OnChatMessageReceived;

	UFUNCTION(Server, Reliable)
	void ServerSendChatMessage(const FString& SenderName, const FString& Message);

	UFUNCTION(Client, Reliable)
	void ClientReceiveChatMessage(const FString& SenderName, const FString& Message,EFPTeamID TeamID);

	// 서버 → 클라이언트: 결과창 표시
	UFUNCTION(Client, Reliable)
	void ClientShowRoundResult();

	// 서버 → 클라이언트: 결과창 숨김
	UFUNCTION(Client, Reliable)
	void ClientHideRoundResult();

	UFUNCTION(Client, Reliable)
	void ClientShowFinalResult();

	UFUNCTION(Client, Reliable)
	void ClientShowPostTravelLoading(TSubclassOf<UFPLoadingWidget> LoadingClass, float Duration, const FString& LoadingText);

	UFUNCTION(Exec)
	void DebugEndRound();

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget>FinalResultWidgetClass;

	// 블루프린트에서 위젯 클래스 지정
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> RoundResultWidgetClass;
	//UI버튼에서 호출할 버튼
	UFUNCTION(BlueprintCallable, Category = "FP|Team")
	void RequestChangeTeam();
	//서버에서 실행될 함수
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestChangeTeam();

	
	UFUNCTION(Client, Reliable)
	void Client_SaveCharacterToInstance(FName CharID, TSubclassOf<APawn> CharClass);
	// 서버에게 캐릭터 복구 요청 (Server RPC)
	UFUNCTION(Server, Reliable)
	void Server_RestoreCharacter(FName SavedID, TSubclassOf<APawn> SavedClass);
	
	virtual void BeginPlay() override;
	virtual void BeginPlayingState() override;
	//위젯이 읽어갈 채팅
	UPROPERTY()
	TArray<FPendingChatMessage> PendingMessages;

private:
	UPROPERTY()
	UUserWidget* RoundResultWidget = nullptr;
	UPROPERTY()
	UUserWidget* FinalResultWidget = nullptr;

	void ApplyGameplayInputMode();
};
