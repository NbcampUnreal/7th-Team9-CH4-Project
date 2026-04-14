#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPLogin.generated.h"

UCLASS()
class FARMSPOPCORN_API UFPLogin : public UUserWidget
{
	GENERATED_BODY()

public:
	//위젯 생성시 호출
	virtual void NativeConstruct() override;

protected:
	//아이디 입력창
	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* IDTextBox;
	//비밀번호 입력창
	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* PasswordTextBox;
	//회원가입 화면에서만 사용되는 비밀번호 확인 입력창
	UPROPERTY(meta=(BindWidgetOptional))
	class UEditableTextBox* ConfirmPasswordTextBox;
	//로그인버튼
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* LoginButton;
	//회원가입버튼
	UPROPERTY(meta = (BindWidget))
	class UButton* SignUpButton;
	//회원가입 확인버튼
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* ConfirmButton;
	//에러 메시지 출력 택스트
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* ERRORTEXT;
	//UPROPERTY(meta = (BindWidgetOptional))
	//class UButton* BackButton;

	//회원가입 UI클래스
	UPROPERTY(EditAnywhere,BlueprintReadwrite, Category = "UI")
	TSubclassOf<class UUserWidget> SignUpWidgetClass;
	//로그인 성공 후 보여줄 닉네임 생성 UI클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> CreateNameWidgetClass;
	//로그인 버튼 클릭 함수
	UFUNCTION()
	void OnLoginClicked();
	//회원가입 버튼 클릭 함수
	UFUNCTION()
	void OnSignUpClicked();
	//회원가입 버튼 클릭 함수
	UFUNCTION()
	void OnConfirmClicked();
	//UFUNCTION()
	//void OnBackClicked();

	//상태 메세지 출력 함수
	void ShowErrorMessage(FString Message, FColor Color = FColor::Black);
	//일정 시간후 에러 메시지 숨기기
	FTimerHandle MessageTimerHandle;
};
