#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Core/FPTeamID.h"
#include "FPGameState.generated.h"

UCLASS()
class FARMSPOPCORN_API AFPGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AFPGameState();
	
#pragma region GameStart //게임 시작
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "FP|State")
	float RemainingTime; 

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "FP|State")
	bool bAllReady; 

	//현재 게임 상태(로비,플레이화면,결과)
	UPROPERTY(ReplicatedUsing = OnRep_GamePhase, BlueprintReadOnly, Category = "FP|State")
	EFPGamePhase GamePhase;

	// 팀 점수
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "FP|Score")
	int32 RedTeamScore = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "FP|Score")
	int32 BlueTeamScore = 0;

	//게임 페이즈
	UFUNCTION()
	void OnRep_GamePhase();
	
#pragma endregion GameStart //게임 시작
	
};
