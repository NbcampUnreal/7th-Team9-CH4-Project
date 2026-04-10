// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/FPUIManagerSubsystem.h"
#include "GameFramework/GameUserSettings.h"

void UFPUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 해상도 고정
	UGameUserSettings* Settings = GEngine->GetGameUserSettings();
	if (Settings)
	{
		Settings->SetScreenResolution(FIntPoint(1920, 1080));
		Settings->SetFullscreenMode(EWindowMode::Fullscreen);
		Settings->ApplySettings(false);
		Settings->SaveSettings();
	}
}
