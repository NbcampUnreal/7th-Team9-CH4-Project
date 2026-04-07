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
#pragma region Game Flow (서버 전용)
	
	void SetGamePhase(EFPGamePhase NewPhase);
	
	virtual void BeginPlay() override;

#pragma endregion
	
#pragma region GameStart //게임 시작
	//게임 페이즈 (UI 연출용)
	UFUNCTION()
	void OnRep_GamePhase();
    
	void HandleCountDownStarted();
	void HandleGameStarted();
	void HandleResultStarted();
    
	UFUNCTION()
	void OnRep_IsReady();
    
	// 현재 게임 상태 (바뀌면 OnRep_GamePhase 실행)
	UPROPERTY(ReplicatedUsing = OnRep_GamePhase, BlueprintReadOnly, Category = "FP|State")
	EFPGamePhase GamePhase;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "FP|State")
	float RemainingTime; 

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "FP|State")
	bool bAllReady = false; 

#pragma endregion GameStart //게임 시작 끝
	
#pragma region GameState //팀 점수
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "FP|Score")
	int32 RedTeamScore = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "FP|Score")
	int32 BlueTeamScore = 0;
#pragma endregion GameState
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
