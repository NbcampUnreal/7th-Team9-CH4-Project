#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "FPChatWidget.generated.h"

UCLASS()
class FARMSPOPCORN_API UFPChatWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	//위젯 생성시 호출
	virtual void NativeConstruct() override;
	//외부에서 채팅을 받아 UI에 표시
	UFUNCTION(BlueprintCallable)
	void ReceiveMessage(const FString& SenderName, const FString& Message);
	
protected:
	//채팅 메시지 목록 스크롤가능
	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ChatScrollBox;
	//채팅 입력창
	UPROPERTY(meta = (BindWidget))
	class UEditableText* ChatInputBox;
	//엔터키 입력시 채팅 입력
	UFUNCTION()
	void OnChatInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

private:
	FTimerHandle MessageCheckTimerHandle;
	//메시지 전송
	void SendMessage(const FString& Message);
//쌓인채팅을UI로 옮김
	UFUNCTION()
	void CheckPendingMessages();
};
