	#pragma once

	#include "CoreMinimal.h"
	#include "Blueprint/UserWidget.h"
	#include "FPCreatenameWidget.generated.h"

	UCLASS()
	class FARMSPOPCORN_API UFPCreatenameWidget : public UUserWidget
	{
		GENERATED_BODY()
	
	public:
		virtual void NativeConstruct() override;
	protected:
		UPROPERTY(meta = (BindWidget))
		class UEditableTextBox* NickNameTextBox; //입력창
		UPROPERTY(meta = (BindWidget))
		class UButton* RandomButton; //랜덤버튼
		UPROPERTY(meta = (BindWidget))
		class UButton* ConfirmButton; //확인버튼
		UPROPERTY(meta = (BindWidget))
		class UButton* CancleButton; //취소버튼
		UPROPERTY(meta = (BindWidgetOptional))
		class UTextBlock* ERRORTEXT;

		UPROPERTY(EditAnywhere, BlueprintReadwrite, Category = "UI")
		TSubclassOf<class UUserWidget> LobbyWidgetClass;

	
		UFUNCTION()
		void OnRandomClicked();
		UFUNCTION()
		void OnConfirmClicked();
		UFUNCTION()
		void OnCancelClicked();
		UFUNCTION()
		void OnNickNameTextChanged(const FText& Text);

		void ShowStatusMessage(FString Message, FColor Color);

		FTimerHandle MessageTimerHandle;
	private:
		FString GenerateRandomNickname();
		bool ValidateNickName(const FString& NickName);
	};
