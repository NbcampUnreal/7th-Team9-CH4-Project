// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FPCharacterDataRow.generated.h"

USTRUCT(BlueprintType)
struct  FARMSPOPCORN_API FFPCharacterDataRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	FString CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	TSoftClassPtr<APawn> CharacterAsset; // 프로젝트에 맞춰 ACharacter 등으로 변경 가능

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	TSoftObjectPtr<UTexture2D> CharacterIcon;
};
