// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPIntroGameMode.generated.h"
 
class UFPIntroWidget;
 
UCLASS()
class FARMSPOPCORN_API AFPIntroGameMode : public AGameModeBase
{
	GENERATED_BODY()
 
public:
	AFPIntroGameMode();
    
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
 
private:
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UFPIntroWidget> IntroWidgetClass;
    
	UPROPERTY()
	UFPIntroWidget* IntroWidget = nullptr;
};
 