#include "UI/FPChatWidget.h"
#include "Player/FPPlayerController.h"
#include "Player/FPPlayerState.h"
#include "Game/FPGameState.h"
#include "Game/FPGameMode.h"
#include "Components/ScrollBox.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
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

void UFPChatWidget::ReceiveMessage(const FString& SenderName, const FString& Message, EFPTeamID TeamID)
{
	//스크롤 박스가 없으면 종료
	if (!ChatScrollBox)
	{
		return;
	}
	UHorizontalBox* ChatRow = NewObject<UHorizontalBox>(this);
	UTextBlock* NicknameText = NewObject<UTextBlock>(this);
	FString NicknameFormatted = FString::Printf(TEXT("[%s]: "), *SenderName);
	NicknameText->SetText(FText::FromString(NicknameFormatted));
	//팀별 메시지 색상
	FLinearColor TeamColor = FLinearColor::White;
	if (TeamID == EFPTeamID::TeamRed) TeamColor = FLinearColor(1.f, 0.4f, 0.4f);
	else if (TeamID == EFPTeamID::TeamBlue) TeamColor = FLinearColor(0.4f, 0.4f, 1.f);
	NicknameText->SetColorAndOpacity(FSlateColor(TeamColor));
	// 3. 메시지 내용 텍스트 생성 (흰색)
	UTextBlock* MessageText = NewObject<UTextBlock>(this);
	MessageText->SetText(FText::FromString(Message));
	MessageText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	MessageText->SetAutoWrapText(true); // 줄바꿈 허용

	// 4. Horizontal Box에 순서대로 추가
	ChatRow->AddChildToHorizontalBox(NicknameText);
	UHorizontalBoxSlot* MsgSlot = ChatRow->AddChildToHorizontalBox(MessageText);

	// 메시지 내용이 길어질 경우를 대비해 슬롯 설정 (선택 사항)
	if (MsgSlot)
	{
		MsgSlot->SetSize(ESlateSizeRule::Fill);
	}

	// 5. 스크롤 박스에 최종 한 줄 추가
	ChatScrollBox->AddChild(ChatRow);
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
		ReceiveMessage(Chat.SenderName, Chat.Message, Chat.TeamID);
	}

	FPC->PendingMessages.Empty();
}
