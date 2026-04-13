#include "UI/FPChatWidget.h"
#include "UI/FPLobbyPlayerController.h"
#include "Components/ScrollBox.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "FPUIManagerSubsystem.h"

void UFPChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogTemp, Warning, TEXT("ChatWidget NativeConstruct 호출됨"));

	//엔터키 입력시 바인딩
	if (ChatInputBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChatInputBox 바인딩 성공"));
		ChatInputBox->OnTextCommitted.AddDynamic(this, &UFPChatWidget::OnChatInputCommitted);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ChatInputBox 바인딩 실패"));
	}
	GetWorld()->GetTimerManager().SetTimer(
		MessageCheckTimerHandle,
		this,
		&UFPChatWidget::CheckPendingMessages,
		0.1f,
		true
	);
	UE_LOG(LogTemp, Warning, TEXT("타이머 시작됨"));
}

void UFPChatWidget::ReceiveMessage(const FString& SenderName, const FString& Message)
{
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
	//0.1초마다 새로운 메시지 체크
	GetWorld()->GetTimerManager().SetTimer(
		MessageCheckTimerHandle,
		this,
		&UFPChatWidget::CheckPendingMessages,
		0.1f,
		true
	);
}

void UFPChatWidget::OnChatInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	UE_LOG(LogTemp, Warning, TEXT("커밋 타입: %d"), (int32)CommitMethod);
	//엔터키로 전송시 처리
	if (CommitMethod != ETextCommit::OnEnter)return;
	FString Message = Text.ToString().TrimStartAndEnd();
	UE_LOG(LogTemp, Warning, TEXT("메시지: %s"), *Message);
	//빈 메시지 무시
	if (Message.IsEmpty()) return;
	SendMessage(Message);
	//입력창 초기화
	if (ChatInputBox)
		ChatInputBox->OnTextCommitted.RemoveAll(this);
	ChatInputBox->SetText(FText::GetEmpty());
	ChatInputBox->OnTextCommitted.AddDynamic(this, &UFPChatWidget::OnChatInputCommitted);

}

void UFPChatWidget::SendMessage(const FString& Message)
{
	//내 닉네임 가져오기
	UFPUIManagerSubsystem* Storage = GetGameInstance()->GetSubsystem<UFPUIManagerSubsystem>();
	if (!Storage) return;
	FString SenderName = Storage->SavedNickName;
	UE_LOG(LogTemp, Warning, TEXT("전송자: %s / 메시지: %s"), *SenderName, *Message);


	//서버에 메시지 전송
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		AFPLobbyPlayerController* FPC = Cast<AFPLobbyPlayerController>(PC);
		if (FPC)
			UE_LOG(LogTemp, Warning, TEXT("컨트롤러 캐스트 성공"));
		FPC->ServerSendChatMessage(SenderName, Message);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("컨트롤러 캐스트 실패"));
	}
}
void UFPChatWidget::CheckPendingMessages()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	AFPLobbyPlayerController* FPC = Cast<AFPLobbyPlayerController>(PC);
	if (!FPC)
	{
		UE_LOG(LogTemp, Warning, TEXT("LobbyController 캐스트 실패"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("PendingMessages 크기: %d"), FPC->PendingMessages.Num());
	for (auto& Pair : FPC->PendingMessages)
		ReceiveMessage(Pair.Key, Pair.Value);

	FPC->PendingMessages.Empty();
}
