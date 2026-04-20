// Fill out your copyright notice in the Description page of Project Settings.


#include "FPIntroController.h"
#include "UI/FPIntroWidget.h"
#include "Engine/World.h"

// Sets default values
AFPIntroController::AFPIntroController()
{
	
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AFPIntroController::BeginPlay()
{
	Super::BeginPlay();
	
	if (!IsLocalController())
	{
		return;
	}

	// 위젯 생성 (로컬 컨트롤러 기준 1회)
	if (IntroWidgetClass && !IntroWidget)
	{
		IntroWidget = CreateWidget<UFPIntroWidget>(this, IntroWidgetClass);
		if (IntroWidget)
		{
			IntroWidget->AddToViewport(0);
		}
	}

	// 마우스 및 입력 모드 설정
	if (IntroWidget)
	{
		bShowMouseCursor = true;
		DefaultMouseCursor = EMouseCursor::Default;

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(IntroWidget->TakeWidget());
		SetInputMode(InputMode);
	}
}

void AFPIntroController::Server_StartLobbyTravel_Implementation()
{
	Server_TravelToLevel(TEXT("/Game/Maps/L_NickNameCreate"));
}

void AFPIntroController::Server_TravelToLevel_Implementation(const FString& LevelPath)
{
	if (LevelPath.IsEmpty())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(LevelPath + TEXT("?listen"), true);
	}
}
