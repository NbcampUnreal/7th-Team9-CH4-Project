#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPLogin.generated.h"

UCLASS()
class FARMSPOPCORN_API UFPLogin : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* IDTextBox;
	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* PasswordTextBox;
	UPROPERTY(meta=(BindWidgetOptional))
	class UEditableTextBox* ConfirmPasswordTextBox;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* LoginButton;
	UPROPERTY(meta = (BindWidget))
	class UButton* SignUpButton;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* ConfirmButton;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* ERRORTEXT;
	//UPROPERTY(meta = (BindWidgetOptional))
	//class UButton* BackButton;

	UPROPERTY(EditAnywhere,BlueprintReadwrite, Category = "UI")
	TSubclassOf<class UUserWidget> SignUpWidgetClass;

	UFUNCTION()
	void OnLoginClicked();
	UFUNCTION()
	void OnSignUpClicked();
	UFUNCTION()
	void OnConfirmClicked();
	//UFUNCTION()
	//void OnBackClicked();
	void ShowErrorMessage(FString Message, FColor Color = FColor::Black);

	FTimerHandle MessageTimerHandle;
};
