#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Core/FPTeamID.h"
#include "FPGameMode.generated.h"

class AFPPlayerState;
UCLASS()
class FARMSPOPCORN_API AFPGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	AFPGameMode();
	
	virtual void PostLogin(APlayerController* NewPlayer);
	
	UPROPERTY(ReplicatedUsing = OnRep_CustomPlayerName)
	FString CustomPlayerName;
	UFUNCTION()
	void OnRep_CustomPlayerName();
	UFUNCTION()
	void OnRep_IsReady();
	
#pragma region Team & Avatar // 팀 & 아바타
public:
	// 플레이어에게 팀을 배정함
	void AssignTeam(AController* Player);
	// 플레이어의 팀을 교체함
	UFUNCTION(BlueprintCallable, Category="Team")
	void ChangeTeam(AController* Player);	
	
	void UpdateTeamCounts();
	// 팀구성 - 초기에 None으로 설정 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Team")
	EFPTeamID TeamID = EFPTeamID::None;
	
	int32 RedTeamCount = 0;
	int32 BlueTeamCount = 0;
protected:
	
	// 무작위 동물 아바타를 스폰하고 플레이어가 조종(빙의)하게 함
	void SpawnAndPossessRandomAvatar(AController* Player);
	
	// 블루프린트에서 동물 캐릭터 클래스들을 등록할 배열
	UPROPERTY(EditDefaultsOnly, Category = "FT|GameRules")
	TArray<TSubclassOf<APawn>> AvatarClasses;
	
	UPROPERTY(EditDefaultsOnly, Category = "FT|GameRules")
	int32 MaxTeams = 2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FP|GameRules")
	int32 MaxPlayersPerTeam = 3;
	
	
#pragma endregion Team & Avatar // 팀 & 아바타 마지막
	
#pragma region GameStart // 게임시작
public:
	void ReStarted();
	// 플레이어가  준비되었는지 확인
	void CheckAllPlayersReady();
	// 모두 준비되었을 때 게임 시작을 처리
	void StartGameCountdown();	
protected:
	bool BGameStarted = false;
#pragma endregion GameStart // 게임시작 마지막
	
};
