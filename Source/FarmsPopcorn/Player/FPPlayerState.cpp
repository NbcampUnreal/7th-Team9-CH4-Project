#include "Player/FPPlayerState.h"
#include "FPPlayerCharacter.h"
#include "Game/FPGameInstance.h"
#include "Game/FPGameMode.h"
#include "Game/FPGameState.h"
#include "Net/UnrealNetwork.h"

AFPPlayerState::AFPPlayerState()
	:CustomPlayerName("NoneName")
{
	CharacterIndex = 0;
}

void AFPPlayerState::BeginPlay()
{
	Super::BeginPlay();
	if (GetNetMode() != NM_DedicatedServer) // 클라이언트에서만 수행
	{
		if (UFPGameInstance* GI = GetGameInstance<UFPGameInstance>())
		{
			// AssignedCharacterClass가 없을 때만 가져오도록 방어 코드 추가
			if (!AssignedCharacterClass)
			{
				CustomPlayerName = GI->SaveNickName;
				AssignedCharacterClass = GI->SaveCharacterClass;
				AssignedCharacterID = GI->SaveCharacterID;
			}
		}
	}

}
void AFPPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	AFPPlayerState* NewPS = Cast<AFPPlayerState>(PlayerState);
	if (NewPS)
	{
		NewPS->AssignedCharacterID = this->AssignedCharacterID;
		NewPS->AssignedCharacterClass = this->AssignedCharacterClass;
		NewPS->AssignedCharacterName = this->AssignedCharacterName;
		NewPS->AssignedCharacterIcon = this->AssignedCharacterIcon;
		NewPS->TeamID = this->TeamID;
		NewPS->CharacterIndex = this->CharacterIndex;
	}
}

void AFPPlayerState::ServerSetCharacterIndex_Implementation(int32 NewIndex)
{
	CharacterIndex = NewIndex;

	UE_LOG(LogTemp, Warning, TEXT("ServerSetCharacterIndex: %s -> %d"),
		*GetPlayerName(), CharacterIndex);
}

void AFPPlayerState::Server_SetReady_Implementation(bool bNewReadyState)
{
	bIsReady = bNewReadyState;
    
	UE_LOG(LogTemp, Warning, TEXT("플레이어 %s의 준비 상태: %s"), 
		*GetPlayerName(), bNewReadyState ? TEXT("준비") : TEXT("대기"));
    
	// 기존 타이머가 있으면 제거
	if (ReadyCheckDelayHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(ReadyCheckDelayHandle);
	}
    
	// 네트워크 동기화를 위해 딜레이 추가
	GetWorld()->GetTimerManager().SetTimer(
		ReadyCheckDelayHandle,
		[this]()
		{
			AFPGameMode* GM = Cast<AFPGameMode>(GetWorld()->GetAuthGameMode());
			if (GM)
			{
				GM->CheckAllPlayersReady();
			}
		},
		0.2f,  // 200ms 딜레이
		false
	);
}

void AFPPlayerState::OnRep_CustomPlayerName()
{
	// 컨트롤러에서 받은 이름을 캐릭터에게 전달
	APawn* OwningPawn =GetPawn();
	if (IsValid(OwningPawn))
	{
		if (AFPPlayerCharacter* Player = Cast<AFPPlayerCharacter>(OwningPawn))
		{
			Player->CurrentName = CustomPlayerName;
		}
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
	DOREPLIFETIME(AFPPlayerState, CharacterIndex);
}