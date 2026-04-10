#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPPlayerSlotWidget.generated.h"

UCLASS()
class FARMSPOPCORN_API UFPPlayerSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	//슬롯에 플레이어 닉네임 설정
	UFUNCTION(BlueprintCallable)
	void SetPlayerName(const FString& InName);
	//슬롯에 플레이어 아이콘 설정
	UFUNCTION(BlueprintCallable)
	void SetCharacterImage(UTexture2D* InTexture);

protected:
	//캐릭터 아이콘
	UPROPERTY(BlueprintReadwrite, meta = (BindWidget))
	class UImage* CharacterImage;
	//플레이어 닉네임 텍스트
	UPROPERTY(BlueprintReadwrite, meta = (BindWidget))
	class UTextBlock* PlayerNameText;
};
