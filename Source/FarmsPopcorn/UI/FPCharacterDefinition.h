#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FPCharacterDefinition.generated.h"

UCLASS(BlueprintType)
class FARMSPOPCORN_API UFPCharacterDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FName CharacterID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FText CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	TSoftObjectPtr<UTexture2D> CharacterIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	TSubclassOf<APawn> CharacterClass;
};
