#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/FPTeamID.h"
#include "FPLobbyCenterSlotWidget.generated.h"

UCLASS()
class FARMSPOPCORN_API UFPLobbyCenterSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetPlayerInfo(const FString& InName, bool bIsReady, UTexture2D* IconTexture, EFPTeamID TeamID);

	UFUNCTION(BlueprintCallable)
	void SetEmpty();

protected:
	UPROPERTY(meta = (BindWidget))
	class UImage* Img_Character;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_PlayerName;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_ReadyState;

	UPROPERTY(meta = (BindWidgetOptional))
	class UBorder* Bg_TeamColor;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_TeamLabel;

};
