#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include  "Core/FPTeamID.h"
#include "FPPlayerState.generated.h"

UCLASS()
class FARMSPOPCORN_API AFPPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AFPPlayerState();
	
	virtual void BeginPlay() override;
	UFUNCTION(Server, Reliable)
	void Server_SetReady(bool bNewReadyState);
	
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "PlayerState")
	bool bIsReady = false;	
	
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Team")
	EFPTeamID TeamID = EFPTeamID::None;
#pragma region Character Setting //캐릭터설정
	
	UFUNCTION()
	void OnRep_CustomPlayerName();

	UPROPERTY(ReplicatedUsing = OnRep_CustomPlayerName)
	FString CustomPlayerName;
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Character")
	FName AssignedCharacterID;    
	//  할당된 캐릭터 클래스
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Character")
	TSubclassOf<APawn> AssignedCharacterClass;    
	//  할당된 캐릭터 이름 (UI 표시용)
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Character")
	FString AssignedCharacterName;    
	//  할당된 캐릭터 아이콘 (UI 표시용)
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Character")
	TSoftObjectPtr<UTexture2D> AssignedCharacterIcon;
#pragma endregion 	//캐릭터 이름 설정 끝
	
public:
	UFUNCTION(BlueprintCallable, Category = "Team")
	EFPTeamID GetTeamID() const { return TeamID; }
	
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
