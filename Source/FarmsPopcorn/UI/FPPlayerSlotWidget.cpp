#include "UI/FPPlayerSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UFPPlayerSlotWidget::SetPlayerName(const FString& InName)
{
	if (PlayerNameText)
		PlayerNameText->SetText(FText::FromString(InName));
}

void UFPPlayerSlotWidget::SetCharacterImage(UTexture2D* InTexture)
{
	if (CharacterImage && InTexture)
		CharacterImage->SetBrushFromTexture(InTexture);
}