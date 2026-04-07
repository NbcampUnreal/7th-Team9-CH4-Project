#include "UI/FPLogin.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"


void UFPLogin::NativeConstruct()
{
	Super::NativeConstruct();

	//UI화면 한정으로 마우스 커서 띄우기
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->bShowMouseCursor = true;

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		PC->SetInputMode(InputMode);
	}

	

	//버튼 클릭 바인딩
	if (LoginButton)
	{
		LoginButton->OnClicked.AddDynamic(this, &UFPLogin::OnLoginClicked);
	}
	if (SignUpButton)
	{
		SignUpButton->OnClicked.Clear();
		if (ConfirmPasswordTextBox)
		{
			SignUpButton->OnClicked.AddDynamic(this, &UFPLogin::OnConfirmClicked);//회원가입 창이면 가입로직 연결
		}
		else
		{
			SignUpButton->OnClicked.AddDynamic(this, &UFPLogin::OnSignUpClicked);//로그인창이면 회원가입창 연결
		}
	}
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UFPLogin::OnConfirmClicked);
	}
	//if (BackButton)
	//{
	//	BackButton->OnClicked.AddDynamic(this, &UFPLogin::OnBackClicked);
	//}
}

void UFPLogin::OnLoginClicked()
{
	//아이디 패스워드 입력값
	FString UserID = IDTextBox->GetText().ToString();
	FString UserPW = PasswordTextBox->GetText().ToString();
	//입력값 검사
	if(UserID.IsEmpty() || UserPW.IsEmpty())
	{
		ShowErrorMessage(TEXT("아이디와 비밀번호를 모두 입력해주세요."), FColor::Orange);
		return;
	}
	//테스트 아이디와 비밀번호
	if (UserID == TEXT("sparta") && UserPW == TEXT("1234"))
	{
		ShowErrorMessage(TEXT("로그인 성공! 잠시 후 이동합니다."), FColor::Cyan);
		FTimerHandle LevelTransitionTimer;
		TWeakObjectPtr<UFPLogin> WeakThis(this);

		GetWorld()->GetTimerManager().SetTimer(LevelTransitionTimer, [WeakThis]()
			{
				if (WeakThis.IsValid())
				{
					UGameplayStatics::OpenLevel(WeakThis->GetWorld(), FName("L_NickNameCreate"));
				}
			}, 1.0f, false);

		UGameplayStatics::OpenLevel(GetWorld(), FName("L_NickNameCreate"));
	}
	else
	{
		ShowErrorMessage(TEXT("아이디나 비밀번호가 틀렸습니다."), FColor::Red);
	}
}
//회원가입 창 띄우기
void UFPLogin::OnSignUpClicked()
{
	if (SignUpWidgetClass)
	{
		UUserWidget* SignUpUI = CreateWidget<UUserWidget>(GetWorld(), SignUpWidgetClass);
		if (SignUpUI)
		{
			SignUpUI->AddToViewport();
		}
	}
}
//회원가입 창 전용 함수 
void UFPLogin::OnConfirmClicked()
{
	if (!ConfirmPasswordTextBox||!IDTextBox||!PasswordTextBox) return;

	FString ID = IDTextBox->GetText().ToString(); //아이디입력
	FString PW = PasswordTextBox->GetText().ToString();//비밀번호 입력
	FString ConfirmPW = ConfirmPasswordTextBox->GetText().ToString(); // 비밀번호 확인
	//입력 안했거나 비밀번호 확인이 틀렸을때
	if (ID.IsEmpty() || PW.IsEmpty()||ConfirmPW.IsEmpty())
	{
		ShowErrorMessage(TEXT("모든 항목을 입력해주세요!"), FColor::Red);
		return;
	}

	if (PW != ConfirmPW)
	{
		ShowErrorMessage(TEXT("비밀번호가 일치하지 않습니다."), FColor::Red);
		return;
	}
	ShowErrorMessage(TEXT("회원가입 성공"), FColor::Cyan);
	FTimerHandle CloseTimerHandle;
	//성공시 다음 레벨로 이동
	TWeakObjectPtr<UFPLogin> WeakThis(this);
	GetWorld()->GetTimerManager().SetTimer(CloseTimerHandle, [WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->RemoveFromParent();
			}
		}, 1.0f, false);

}
void UFPLogin::ShowErrorMessage(FString Message, FColor Color)
{
	//에러 메시지 텍스트와 색상 설정 후 출력
	if (ERRORTEXT)
	{
		ERRORTEXT->SetText(FText::FromString(Message));
		ERRORTEXT->SetColorAndOpacity(FSlateColor(FLinearColor(Color)));
		ERRORTEXT->SetVisibility(ESlateVisibility::Visible);
		//3초뒤에 에러메시지 숨김처리
		TWeakObjectPtr<UFPLogin> WeakThis(this);
		GetWorld()->GetTimerManager().SetTimer(MessageTimerHandle, FTimerDelegate::CreateLambda([WeakThis]()
			{
				if (WeakThis.IsValid() && WeakThis->ERRORTEXT)
				{
					WeakThis->ERRORTEXT->SetVisibility(ESlateVisibility::Hidden);
				}
			}), 3.0f, false);
	}
}
//void UFPLogin::OnBackClicked()
//{
//	RemoveFromParent();
//}