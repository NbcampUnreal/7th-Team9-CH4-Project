#include "UI/FPLobbyWidget.h"
#include "Player/FPPlayerController.h"
#include "FPPlayerSlotWidget.h"
#include "FPPlayerListEntryWidget.h"
#include "Player/FPPlayerState.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework//PlayerState.h"
#include "FPUIManagerSubsystem.h"

void UFPLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	//닉네임 표시
	UFPUIManagerSubsystem* Storage = GetGameInstance()->GetSubsystem<UFPUIManagerSubsystem>();
	//내 닉네임 UI에 표시
	if (Storage && DisplayNickName)
	{
		DisplayNickName->SetText(FText::FromString(Storage->SavedNickName));
	}
	//UI 전용 마우스 커서 활성화
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputMode);
	}
	//초기 텍스트설정
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(TEXT("대기중...")));
	}
	if (ReadyButtonText)
	{
		ReadyButtonText->SetText(FText::FromString(TEXT("준비")));
	}
	//준비 버튼 누를시 활성화
	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddDynamic(this, &UFPLobbyWidget::OnReadyClicked);
	}
	//닉네임 서버에 등록
	if (Storage && PC)
	{
		AFPPlayerController* FPC = Cast<AFPPlayerController>(PC);
		if (FPC)
		{
			FPC->ServerSetCustomName(Storage->SavedNickName);
		}
	}
	//서버 접속 2초 뒤 부터 1초마다 플레이어 탐지(동기화 대기)
	GetWorld()->GetTimerManager().SetTimer(
		PlayerCheckTimerHandle,
		this,
		&UFPLobbyWidget::CheckPlayerArray,
		0.2f,
		true,
		2.0f);
}

void UFPLobbyWidget::NativeDestruct()
{
	Super::NativeDestruct();
	//타이머정리(메모리 누수 방지)
	GetWorld()->GetTimerManager().ClearTimer(PlayerCheckTimerHandle);
}

void UFPLobbyWidget::OnPlayerJoined(const FString& PlayerName, int32 CharacterIndex)
{
	//HorizontalBox 캐릭터 슬롯 추가
	if (PlayerSlotWidgetClass && PlayerSpawnBox)
	{
		UFPPlayerSlotWidget* NewSlot = CreateWidget<UFPPlayerSlotWidget>(this, PlayerSlotWidgetClass);
		if (NewSlot)
		{
			//슬롯에 닉네임 + 캐릭터 이미지 설정
			NewSlot->SetPlayerName(PlayerName);
			if (CharacterImages.IsValidIndex(CharacterIndex))
				NewSlot->SetCharacterImage(CharacterImages[CharacterIndex]);
			
			//HorizontalBox에 추가 후 균등 배치
			UHorizontalBoxSlot* HSlot = PlayerSpawnBox->AddChildToHorizontalBox(NewSlot);
			if (HSlot)
			{
				FSlateChildSize FillSize;
				FillSize.SizeRule = ESlateSizeRule::Fill;
				FillSize.Value = 1.0f;
				HSlot->SetSize(FillSize);
				HSlot->SetPadding(FMargin(5.0f));
			}
			//닉네임 기준으로 슬롯에 맵 저장
			SpawnSlotMap.Add(PlayerName, NewSlot);
		}
	}
	//VerticalBox에 플레이어 목록 추가
	if (PlayerListEntryWidgetClass && PlayerListBox)
	{
		UFPPlayerListEntryWidget* NewEntry = CreateWidget<UFPPlayerListEntryWidget>(this, PlayerListEntryWidgetClass);
		if (NewEntry)
		{
			//닉네임 + 초기 준비상태설정
			NewEntry->SetPlayerInfo(PlayerName, false);
			//VerticalBox에 추가 후 간격 설정
			UVerticalBoxSlot* VSlot = PlayerListBox->AddChildToVerticalBox(NewEntry);
			if (VSlot)
				VSlot->SetPadding(FMargin(0.0f, 5.0f));
			//닉네임 기준으로 목록 맵 저장
			ListEntryMap.Add(PlayerName, NewEntry);
		}
	}
}

void UFPLobbyWidget::OnPlayerLeft(const FString& PlayerName)
{
	//슬롯에서 해당 플레이어 제거
	if (UFPPlayerSlotWidget** FoundSlot = SpawnSlotMap.Find(PlayerName))
	{
		(*FoundSlot)->RemoveFromParent();
		SpawnSlotMap.Remove(PlayerName);
	}

	// 목록에서 해당 플레이어 제거
	if (UFPPlayerListEntryWidget** FoundEntry = ListEntryMap.Find(PlayerName))
	{
		(*FoundEntry)->RemoveFromParent();
		ListEntryMap.Remove(PlayerName);
	}
}

void UFPLobbyWidget::OnPlayerReadyChanged(const FString& PlayerName, bool bNewReadyState)
{
	//해당 플레이어의 목록 항목 준비
	if (UFPPlayerListEntryWidget** FoundEntry = ListEntryMap.Find(PlayerName))
	{
		(*FoundEntry)->UpdateReadyState(bNewReadyState);
	}
}

void UFPLobbyWidget::OnReadyClicked()
{
	bIsReady = !bIsReady; //준비상태 취소상태
	//UI 갱신
	if (bIsReady)
	{
		if (StatusText) StatusText->SetText(FText::FromString(TEXT("준비 완료")));
		if (ReadyButtonText) ReadyButtonText->SetText(FText::FromString(TEXT("취소")));
	}
	else
	{
		if (StatusText) StatusText->SetText(FText::FromString(TEXT("대기 중...")));
		if (ReadyButtonText) ReadyButtonText->SetText(FText::FromString(TEXT("준비")));
	}
	//서버에 준비상태 전달
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (AFPPlayerController* FPPC = Cast<AFPPlayerController>(PC))
	{
		FPPC->SetReady(bIsReady);
		UFPUIManagerSubsystem* Storage = GetGameInstance()->GetSubsystem<UFPUIManagerSubsystem>();
		if (Storage && ListEntryMap.Contains(Storage->SavedNickName))
			OnPlayerReadyChanged(Storage->SavedNickName, bIsReady);
	}
}

void UFPLobbyWidget::UpdateReadyStatus(bool bNewReadyState)
{
	//텍스트 업데이트
	if (bNewReadyState)
	{
		StatusText->SetText(FText::FromString(TEXT("준비 완료")));
		ReadyButtonText->SetText(FText::FromString(TEXT("취소")));
	}
	else
	{
		StatusText->SetText(FText::FromString(TEXT("대기 중...")));
		ReadyButtonText->SetText(FText::FromString(TEXT("준비")));
	}
}

void UFPLobbyWidget::CheckPlayerArray()
{
	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) return;
	//디버그 로그
	UE_LOG(LogTemp, Warning, TEXT("PlayerArray 크기: %d"), GS->PlayerArray.Num());
	UE_LOG(LogTemp, Warning, TEXT("HasAuthority: %s"), *FString(GetWorld()->GetNetMode() == NM_ListenServer ? TEXT("ListenServer") : TEXT("Client")));
	//현재 접속중인 플레이어 이름
	TArray<FString> CurrentPlayerNames;
	for (APlayerState* PS : GS->PlayerArray)
	{
		if (!PS || PS->IsABot()) continue;

		AFPPlayerState* FPS = Cast<AFPPlayerState>(PS);
		//플레이어 이름 있으면 사용 없으면 NONE
		FString Name = (FPS && !FPS->CustomPlayerName.IsEmpty())
			? FPS->CustomPlayerName
			: PS->GetPlayerName();
		UE_LOG(LogTemp, Warning, TEXT("플레이어 이름: %s"), *Name);
		CurrentPlayerNames.Add(Name);
		//이미 목록에 있는 플레이어 준비상태
		if (FPS && ListEntryMap.Contains(Name))
		{
			OnPlayerReadyChanged(Name, FPS->bIsReady);
		}
	}
	//새로 들어온 플레이어 감지
	for (const FString& Name : CurrentPlayerNames)
	{
		if (!CaChedPlayerNames.Contains(Name))
		{
			OnPlayerJoined(Name, 0);
		}
	}
	//나간 플레이어 감지
	for (const FString& Name : CaChedPlayerNames)
	{
		if (!CurrentPlayerNames.Contains(Name))
		{
			OnPlayerLeft(Name);
		}
	}
	//캐시 업데이트
	CaChedPlayerNames = CurrentPlayerNames;
}
