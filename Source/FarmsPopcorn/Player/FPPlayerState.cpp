#include "Player/FPPlayerState.h"
#include "FPPlayerCharacter.h"
#include "Game/FPGameInstance.h"
#include "Game/FPGameMode.h"
#include "Net/UnrealNetwork.h"

AFPPlayerState::AFPPlayerState()
	:CustomPlayerName("NoneName")
{
	
}

void AFPPlayerState::BeginPlay()
{
	Super::BeginPlay();
	if (UFPGameInstance* GI = GetGameInstance<UFPGameInstance>())
	{
		CustomPlayerName = GI->SaveNickName;
		TeamID = GI->SaveTeamID;
		AssignedCharacterClass = GI->SaveCharacterClass;
		AssignedCharacterID = GI->SaveCharacterID;
	}

}


void AFPPlayerState::Server_SetReady_Implementation(bool bNewReadyState)
{
	bIsReady = bNewReadyState;
	UE_LOG(LogTemp, Warning, TEXT("플레이어 %s의 준비 상태: %s"), 
		*GetPlayerName(), bNewReadyState ? TEXT("준비") : TEXT("대기"));
    
	AFPGameMode* GM = Cast<AFPGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->CheckAllPlayersReady(); 
	}
}

void AFPPlayerState::OnRep_CustomPlayerName()
{
	// 컨트롤러에서 받은 이름을 캐릭터에게 전달
	APawn* OwningPawn =GetPawn();
	if (IsValid(OwningPawn))
	{
		AFPPlayerCharacter* Player = Cast<AFPPlayerCharacter>(OwningPawn);
		Player->CurrentName = CustomPlayerName;
	}
}

void AFPPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);	
	DOREPLIFETIME(AFPPlayerState, CustomPlayerName);
	DOREPLIFETIME(AFPPlayerState, TeamID);
	DOREPLIFETIME(AFPPlayerState, bIsReady);
	DOREPLIFETIME(AFPPlayerState, AssignedCharacterID);
	DOREPLIFETIME(AFPPlayerState, AssignedCharacterClass);
	DOREPLIFETIME(AFPPlayerState, AssignedCharacterName);
	DOREPLIFETIME(AFPPlayerState, AssignedCharacterIcon);
	
}