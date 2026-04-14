#include "UI/FPCreatenameWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "FPUIManagerSubsystem.h"

void UFPCreatenameWidget::NativeConstruct()
{
	Super::NativeConstruct();
	//택스트 변할때마다 유효성검사
	if (NickNameTextBox)
	{
		NickNameTextBox->OnTextChanged.AddDynamic(this, &UFPCreatenameWidget::OnNickNameTextChanged);
	}
	//초기 버튼 상태
	if (ConfirmButton)ConfirmButton->SetIsEnabled(false);
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	//컨트롤러 대신하는 역할(UI한정 마우스 커서 보임)
	if (PC)
	{
		PC->bShowMouseCursor = true;
		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(Mode);
	}

	//닉네임 생성창 버튼
	if (RandomButton)//랜덤 닉네임
		RandomButton->OnClicked.AddDynamic(this, &UFPCreatenameWidget::OnRandomClicked);
	if(ConfirmButton)//닉네임 생성 성공
		ConfirmButton->OnClicked.AddDynamic(this, &UFPCreatenameWidget::OnConfirmClicked);
	if(CancleButton)// 닉네임 생성 실패
		CancleButton->OnClicked.AddDynamic(this, &UFPCreatenameWidget::OnCancelClicked);
}
//랜덤닉네임
void UFPCreatenameWidget::OnRandomClicked()
{
	FString NewName = GenerateRandomNickname();

	if (NickNameTextBox)
	{
		NickNameTextBox->SetText(FText::FromString(NewName));
		OnNickNameTextChanged(FText::FromString(NewName));
	}
}
//닉네임 생성
void UFPCreatenameWidget::OnConfirmClicked()
{
	if (!NickNameTextBox) return;

	//닉네임 가져오기
	FString Nick = NickNameTextBox->GetText().ToString();

	//로컬 저장
	UFPUIManagerSubsystem* Storage = GetGameInstance()->GetSubsystem<UFPUIManagerSubsystem>();
	if (Storage)
	{
		Storage->SavedNickName = Nick;
	}

	//유효성 검사
	if (!ValidateNickName(Nick))
	{
		ShowStatusMessage(TEXT("닉네임 형식이 올바르지 않습니다."), FColor::Red);
		return;
	}
		 ShowStatusMessage(TEXT("닉네임 생성 성공! %s 환영합니다!. 로비로 이동합니다."), FColor::Cyan);
		 FTimerHandle LevelTimer;
		 TWeakObjectPtr<UFPCreatenameWidget> WeakThis(this);
		 GetWorld()->GetTimerManager().SetTimer(LevelTimer, [WeakThis]()
			 {
				 if (WeakThis.IsValid())
				 {
					 WeakThis->GetWorld()->ServerTravel(TEXT("/Game/Maps/L_Lobby?listen"));
				 }
			 }, 1.0f, false);
}
//닉네임 생성 취소
void UFPCreatenameWidget::OnCancelClicked()
{
	RemoveFromParent();
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		FInputModeGameOnly Mode;
		PC->SetInputMode(Mode);
		PC->bShowMouseCursor = false;
	}
}

void UFPCreatenameWidget::OnNickNameTextChanged(const FText& Text)
{
	FString Nick = Text.ToString();
	bool bIsValid = ValidateNickName(Text.ToString());
	FString ErrorMsg = TEXT("");
	//닉네임 길이 검사 2~6자
	if (Nick.Len() < 2)
	{
		bIsValid = false;
		ErrorMsg = TEXT("닉네임이 너무 짧습니다 (2자 이상)");
	}
	else if (Nick.Len() > 6)
	{
		bIsValid = false;
		ErrorMsg = TEXT("닉네임이 너무 깁니다 (6자 이하)");
	}
	//닉네임 자릿수 결과에따라 확인버튼 활성화 비활성화
	if (ConfirmButton)
	{
		ConfirmButton->SetIsEnabled(bIsValid);
	}
	//닉네임 조건 이 맞지않으면 에러메시지 출력
	if (!bIsValid && !Nick.IsEmpty())
	{
		ShowStatusMessage(ErrorMsg, FColor::Red);
	}
	else if (bIsValid && !Nick.IsEmpty())
	{
		// 유효하다면 안내 문구 숨기기
		if (ERRORTEXT) ERRORTEXT->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UFPCreatenameWidget::ShowStatusMessage(FString Message, FColor Color)
{
	//에러 메시지 텍스트와 색상 설정 후 출력
	if (ERRORTEXT)
	{
		GetWorld()->GetTimerManager().ClearTimer(MessageTimerHandle);

		ERRORTEXT->SetText(FText::FromString(Message));
		ERRORTEXT->SetColorAndOpacity(FSlateColor(FLinearColor(Color)));
		ERRORTEXT->SetVisibility(ESlateVisibility::Visible);
		//3초뒤에 에러메시지 숨김처리
		TWeakObjectPtr<UFPCreatenameWidget> WeakThis(this);
		GetWorld()->GetTimerManager().SetTimer(MessageTimerHandle, [WeakThis]()
			{
				if (WeakThis.IsValid() && WeakThis->ERRORTEXT)
				{
					WeakThis->ERRORTEXT->SetVisibility(ESlateVisibility::Hidden);
				}
			}, 3.0f, false);
	}
}


FString UFPCreatenameWidget::GenerateRandomNickname()
{
	//랜덤 닉네임 목록(AI 추천 닉네임)
	TArray<FString> Names =
	{
		TEXT("젤리곰"),
		TEXT("코코호랑이"),
		TEXT("별빛펭귄"),
		TEXT("쿠키몽키"),
		TEXT("핑크여우"),
		TEXT("초코토끼")
	};
	int32 Index = FMath::RandRange(0, Names.Num() - 1);
	return Names[Index];
}
//닉네임 글자수 설정
bool UFPCreatenameWidget::ValidateNickName(const FString& NickName)
{
	return true;
}
