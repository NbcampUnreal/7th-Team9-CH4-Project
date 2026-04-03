#pragma once

#include "CoreMinimal.h"
#include "FPTeamID.generated.h"

UENUM(BlueprintType)
enum class EFPTeamID : uint8
{
	None UMETA(DisplayName = "None"),
	TeamRed UMETA(DisplayName = "Team Red"),
	TeamBlue UMETA(DisplayName = "Team Blue")
};
UENUM(BlueprintType)
enum class EFPGamePhase : uint8
{
	Lobby UMETA(DisplayName = "Lobby"),
	CountDown UMETA(DisplayName = "CountDown"),
	InGame UMETA(DisplayName = "In Game"),
	Result UMETA(DisplayName = "Result")
};