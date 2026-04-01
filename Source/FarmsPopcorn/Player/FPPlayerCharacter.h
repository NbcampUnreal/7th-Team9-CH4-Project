#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPPlayerCharacter.generated.h"

UCLASS()
class FARMSPOPCORN_API AFPPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFPPlayerCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
