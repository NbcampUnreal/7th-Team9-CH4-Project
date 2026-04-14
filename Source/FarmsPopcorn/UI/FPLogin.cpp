#include "UI/FPLogin.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"


void UFPLogin::NativeConstruct()
{
	Super::NativeConstruct();

	//UI화면 한정으로 마우스 커서 띄우기
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		PC = GetWorld()->GetFirstPlayerController();
	}
	//UI전용 입력 모드 설정
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
		LoginButton->OnClicked.RemoveDynamic(this, &UFPLogin::OnLoginClicked);
		LoginButton->OnClicked.AddDynamic(this, &UFPLogin::OnLoginClicked);
	}
	//회원가입 버튼은 현재 화면 성격에 따라 다른 함수에 연결
	if (SignUpButton)
	{
		SignUpButton->OnClicked.RemoveDynamic(this, &UFPLogin::OnConfirmClicked);
		SignUpButton->OnClicked.RemoveDynamic(this, &UFPLogin::OnSignUpClicked);
		
		//ConfirmPasswordTextBox가 있으면 회원가입 화면이라고 판단
		if (ConfirmPasswordTextBox)
		{
			SignUpButton->OnClicked.AddDynamic(this, &UFPLogin::OnConfirmClicked);//회원가입 창이면 가입로직 연결
		}
		else
		{
			SignUpButton->OnClicked.AddDynamic(this, &UFPLogin::OnSignUpClicked);//로그인창이면 회원가입창 연결
		}
	}
	//Confirm 버튼이 있으면 회원가입 확인 함수 바인딩
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UFPLogin::OnConfirmClicked);
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
	FString UserID = IDTextBox ? IDTextBox->GetText().ToString() : TEXT("");
	FString UserPW = PasswordTextBox ? PasswordTextBox->GetText().ToString() : TEXT("");

	//입력값 검사
	if (UserID.IsEmpty() || UserPW.IsEmpty())
	{
		ShowErrorMessage(TEXT("아이디와 비밀번호를 모두 입력해주세요."), FColor::Orange);
		return;
	}
	//테스트 아이디와 비밀번호
	if (UserID == TEXT("sparta") && UserPW == TEXT("1234"))
	{
		ShowErrorMessage(TEXT("로그인 성공! 잠시 후 이동합니다."), FColor::Cyan);
		APlayerController* PC = GetOwningPlayer();
		if (!PC)
		{
			PC = GetWorld()->GetFirstPlayerController();
		}
		UE_LOG(LogTemp, Warning, TEXT("Login Success"));
		//닉네임 생성 UI클래스나 컨트롤러가 없으면 종료
		if (!CreateNameWidgetClass || !PC)
		{
			UE_LOG(LogTemp, Error, TEXT("CreateWidget Failed"));
			return;
		}
		//닉네임 생성 위젯 생성후 화면에 표시
		UUserWidget* CreateNameUI = CreateWidget<UUserWidget>(PC, CreateNameWidgetClass);
		if (CreateNameUI)
		{
			UE_LOG(LogTemp, Warning, TEXT("Widget Created"));

			CreateNameUI->AddToViewport(100);
			UE_LOG(LogTemp, Warning, TEXT("Added to Viewport"));
			//현재 로그인 위젯 제거
			RemoveFromParent();
			UE_LOG(LogTemp, Warning, TEXT("Login UI Removed"));
		}
		return;
	}
	//로그인 실패 메시지
	ShowErrorMessage(TEXT("아이디나 비밀번호가 틀렸습니다."), FColor::Red);
}
//회원가입 창 띄우기
void UFPLogin::OnSignUpClicked()
{
	//회원가입 UI클래스가 없으면 종료
	if (!SignUpWidgetClass)
	{
		return;
	}
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		PC = GetWorld()->GetFirstPlayerController();
	}
	if (!PC)
	{
		return;
	}
	//회원가이비 위젯 생성후 화면에 표시
	UUserWidget* SignUpUI = CreateWidget<UUserWidget>(PC, SignUpWidgetClass);
	if (SignUpUI)
	{
		SignUpUI->AddToViewport();
	}
}
//회원가입 창 전용 함수 
void UFPLogin::OnConfirmClicked()
{
	//회원가입 입력창이 없으면 종료
	if (!ConfirmPasswordTextBox||!IDTextBox||!PasswordTextBox) return;
	//입력값 가져오기
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
	//성공 메시지 후 웨젯 닫기
	FTimerHandle CloseTimerHandle;
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
	if (!ERRORTEXT)
	{
		return;
	}
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

//void UFPLogin::OnBackClicked()
//{
//	RemoveFromParent();
//}