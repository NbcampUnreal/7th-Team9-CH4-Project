#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Core/FPTeamID.h"
#include "Data/FPCharacterDataRow.h"
#include "FPGameMode.generated.h"

class AFPPlayerState;
UCLASS()
class FARMSPOPCORN_API AFPGameMode : public AGameMode
{
    GENERATED_BODY()
    
public:
    AFPGameMode();
    
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void BeginPlay() override;
    virtual void HandleSeamlessTravelPlayer(AController*& C) override;

#pragma region Team & Avatar // 팀 & 아바타
public:
    void AssignCharacterToPlayer(APlayerController* PlayerController);
    
    // 플레이어에게 팀을 배정함
    void AssignTeam(AController* Player);
    
    // 플레이어의 팀을 교체함, 로비제작시 팀 교체 버튼 
    UFUNCTION(BlueprintCallable, Category="Team") 
    void ChangeTeam(AController* Player);
    
    void CheckPlayerAvater();  
    void UpdateTeamCounts();
    
    // 팀구성 - 초기에 None으로 설정 
    UPROPERTY(BlueprintReadWrite, Category = "Team")
    EFPTeamID TeamID = EFPTeamID::None;
    
    // 게임 중 선택된 캐릭터들을 기록 (중복 방지용)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
    TArray<FName> UsedCharacterIDs;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
    UDataTable* CharacterDataTable;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    FVector RedTeamSpawnPoint = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    FVector BlueTeamSpawnPoint = FVector::ZeroVector;
    
    int32 RedTeamCount = 0;
    int32 BlueTeamCount = 0;
    
protected:
    
    virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

    // 동물 캐릭터 클래스들을 등록
    UPROPERTY(EditDefaultsOnly, Category = "FT|GameRules")
    TArray<TSubclassOf<APawn>> AvatarClasses;
    
    UPROPERTY(EditDefaultsOnly, Category = "FT|GameRules")
    int32 MaxTeams = 2;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FP|GameRules")
    int32 MaxPlayersPerTeam = 3;
    
    
#pragma endregion Team & Avatar // 팀 & 아바타 마지막
    
#pragma region GameStart // 게임시작
public:
    // 플레이어가  준비되었는지 확인
    void CheckAllPlayersReady();
    // 모두 준비되었을 때 게임 시작을 처리
    void StartGameCountdown(); 
    // 레디 상태확인
    bool ReadyPlayerCheck();
    void DelayedReadyPlayerCheck();

protected:
    FTimerHandle CountDownHandle; // 3초를 잴 타이머
    FTimerHandle ReadyCheckHandle; // 레디체크 타이머
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameSettings")
    TSoftObjectPtr<UWorld> NextLevel; // level 이동
    void ExecuteMapTravel();      // 3초 뒤에 실제로 맵을 넘길 함수
    void ResetAvatarSelection(); //아바타 리셋 (게임 재시작시)
    #pragma endregion GameStart // 게임시작 마지막
    
#pragma region Score //팀 점수
public:    
    //점수 올리기
    void AddScoreToTeam(EFPTeamID InTeamID, int32 ScoreAmount);
	
    //레드팀 점수
    int32 RedTeamScore = 0;
    //블루팀 점수
    int32 BlueTeamScore = 0;
    
    
#pragma endregion 
};