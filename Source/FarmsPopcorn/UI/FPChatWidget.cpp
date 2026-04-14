#include "UI/FPChatWidget.h"
#include "Player/FPPlayerController.h"
#include "Components/ScrollBox.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "FPUIManagerSubsystem.h"

void UFPChatWidget::NativeConstruct()
{
	Super::NativeConstruct();
	//입력창이 이으면 엔터 입력
	if (ChatInputBox)
	{
		ChatInputBox->OnTextCommitted.RemoveAll(this);
		ChatInputBox->OnTextCommitted.AddDynamic(this, &UFPChatWidget::OnChatInputCommitted);
	}
	//델리게이트로 바인딩
	if (AFPPlayerController* FPC = Cast<AFPPlayerController>(GetOwningPlayer()))
	{
		FPC->OnChatMessageReceived.RemoveAll(this);
		FPC->OnChatMessageReceived.AddUObject(this, &UFPChatWidget::ReceiveMessage);
	}
}

void UFPChatWidget::ReceiveMessage(const FString& SenderName, const FString& Message)
{
	//스크롤 박스가 없으면 종료
	if (!ChatScrollBox)
	{
		return;
	}
	//메시지 텍스트 생성
	UTextBlock* NewMessage = NewObject<UTextBlock>(this);
	if (!NewMessage)
	{
		return;
	}
	//메시지 형식: [닉네임]: 메시지
	FString FullMessage = FString::Printf(TEXT("[%s]: %s"), *SenderName, *Message);
	NewMessage->SetText(FText::FromString(FullMessage));

	//폰트 크기 설정
	FSlateFontInfo FontInfo = NewMessage->GetFont();
	FontInfo.Size = 14;
	NewMessage->SetFont(FontInfo);
	//텍스트 색상 설정
	NewMessage->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	//자동 줄바꿈
	NewMessage->SetAutoWrapText(true);


	//스크롤 추가
	ChatScrollBox->AddChild(NewMessage);
	//최신 메시지로 스크롤
	ChatScrollBox->ScrollToEnd();
}

void UFPChatWidget::OnChatInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	UE_LOG(LogTemp, Warning, TEXT("커밋 타입: %d"), (int32)CommitMethod);
	//엔터키로 전송시 처리
	if (CommitMethod != ETextCommit::OnEnter)
	{
		return;
	}
	//앞 뒤 공백 제거
	const FString Message = Text.ToString().TrimStartAndEnd();
	//빈 메시지는 무시
	if (Message.IsEmpty())
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("메시지: %s"), *Message);
	//메시지 전송
	SendMessage(Message);
	//입력창 초기화
	if (ChatInputBox)
	{
		ChatInputBox->SetText(FText::GetEmpty());
		ChatInputBox->SetKeyboardFocus();
	}
}

void UFPChatWidget::SendMessage(const FString& Message)
{
	//내 닉네임 가져오기
	UFPUIManagerSubsystem* Storage = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFPUIManagerSubsystem>() : nullptr;
	if (!Storage)
	{
		return;
	}
	//위젯의 소유 플레이어 호출
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("OwningPlayer 없음"));
		return;
	}


	//PlayerCOntroller로 캐스팅
	AFPPlayerController* FPC = Cast<AFPPlayerController>(PC);
	if (!FPC)
	{
		UE_LOG(LogTemp, Warning, TEXT("FPPlayerComtroller 캐스트 실패"));
		return;
	}
	//서버로 채팅 전송
	FPC->ServerSendChatMessage(Storage->SavedNickName, Message);
}
void UFPChatWidget::CheckPendingMessages()
{
	//윚젯의 소유 플레이어 호출
	APlayerController* PC = GetOwningPlayer();
	AFPPlayerController* FPC = Cast<AFPPlayerController>(PC);
	if (!FPC)
	{
		return;
	}
	//메시지 하나씩 UI에 표시
	for (const FPendingChatMessage& Chat : FPC->PendingMessages)
	{
		ReceiveMessage(Chat.SenderName, Chat.Message);
	}

	FPC->PendingMessages.Empty();
}
