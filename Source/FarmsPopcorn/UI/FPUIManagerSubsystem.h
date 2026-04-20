
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FPUIManagerSubsystem.generated.h"

UCLASS()
class FARMSPOPCORN_API UFPUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(BlueprintReadWrite, Category = "Player Data")
	FString SavedNickName;

	UPROPERTY()
	TObjectPtr<class UUserWidget> PersistentScoreWidget;
};
