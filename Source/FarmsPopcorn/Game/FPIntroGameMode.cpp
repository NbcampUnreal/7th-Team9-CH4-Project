// Fill out your copyright notice in the Description page of Project Settings.


// IntroGameMode.cpp
#include "FPIntroGameMode.h"
#include "UI/FPIntroWidget.h"
#include "Kismet/GameplayStatics.h"
 
AFPIntroGameMode::AFPIntroGameMode()
{
	// 멀티플레이 지원
	bUseSeamlessTravel = true;
}
 
void AFPIntroGameMode::BeginPlay()
{
	Super::BeginPlay();
    
	UE_LOG(LogTemp, Warning, TEXT("=== AFPIntroGameMode BeginPlay ==="));
}
 
void AFPIntroGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
    
	if (!NewPlayer)
	{
		return;
	}
    
	UE_LOG(LogTemp, Warning, TEXT("✓ PostLogin Called for Player: %s"), *NewPlayer->GetName());
}
