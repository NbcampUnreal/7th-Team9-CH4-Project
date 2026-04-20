// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPIntroController.generated.h"

class UFPIntroWidget;
UCLASS()
class FARMSPOPCORN_API AFPIntroController : public APlayerController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFPIntroController();

public:
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void Server_StartLobbyTravel();

	UFUNCTION(Server, Reliable)
	void Server_TravelToLevel(const FString& LevelPath);

private:
	UPROPERTY()
	UFPIntroWidget* IntroWidget = nullptr;
    
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UFPIntroWidget> IntroWidgetClass;
};
