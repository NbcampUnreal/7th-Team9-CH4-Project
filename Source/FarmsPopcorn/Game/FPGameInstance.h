// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Core/FPTeamID.h"
#include "Data/FPCharacterDataRow.h"
#include "FPGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class FARMSPOPCORN_API UFPGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFPGameInstance();
	
	virtual void Init() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GameData")
	FString SaveNickName; //플레이어네임
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GameData")
	int32 SaveCurrentRound = 0;
	//팀스코어
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GameData")
	int32 SaveRedScore = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GameData")
	int32 SaveBlueScore = 0;
	
	//팀 정보
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GameData")
	EFPTeamID SaveTeamID = EFPTeamID::None;

	UPROPERTY()
	TSubclassOf<APawn> SaveCharacterClass; // 캐릭터 모델 저장

	UPROPERTY()
	FName SaveCharacterID; //캐릭터 ID
	UPROPERTY()
	FString SaveCharacterName;
	UPROPERTY()
	TSoftObjectPtr<UTexture2D> SaveCharacterIcon;
};
