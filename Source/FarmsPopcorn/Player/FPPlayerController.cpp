#include "Player/FPPlayerController.h"
#include "FPPlayerState.h"

void AFPPlayerController::ServerSetCustomName_Implementation(const FString& NewName)
{
	// UI에서 받은 이름을 플레이어 스테이트에게 전달
	if (NewName.IsEmpty())return;
	if (auto APlayerState = GetPlayerState<AFPPlayerState>())
	{
		APlayerState->CustomPlayerName = NewName;
	}
}
