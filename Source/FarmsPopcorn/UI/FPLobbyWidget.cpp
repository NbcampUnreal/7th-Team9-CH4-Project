#include "UI/FPLobbyWidget.h"
#include "UI/FPLobbyCenterSlotWidget.h"
#include "Player/FPPlayerController.h"
#include "Game/FPGameState.h"
#include "UI/FPPlayerSlotWidget.h"
#include "UI/FPPlayerListEntryWidget.h"
#include "Player/FPPlayerState.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"
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

	if (Btn_Next)
	{
		Btn_Next->OnClicked.AddDynamic(this, &UFPLobbyWidget::HandleNextClicked);
	}
	if (Btn_Prev)
	{
		Btn_Prev->OnClicked.AddDynamic(this, &UFPLobbyWidget::HandlePrevClicked);
	}
	APlayerController* InitPC = GetWorld()->GetFirstPlayerController();
	if (InitPC)
	{
		AFPPlayerState* InitPS = InitPC->GetPlayerState<AFPPlayerState>();
		if (InitPS)
		{
			CurrentPreviewIndex = InitPS->CharacterIndex;
		}

		if (ImgCharPreview && CharacterImages.IsValidIndex(CurrentPreviewIndex))
		{
			ImgCharPreview->SetBrushFromTexture(CharacterImages[CurrentPreviewIndex]);
		}
	}

}
int32 UFPLobbyWidget::GetAvailableIndex(int32 CurrentIdx, bool bForward)
{
	int32 MaxCount = CharacterImages.Num();
	if (MaxCount <= 0) return 0;

	AFPGameState* MyGS = Cast<AFPGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (!MyGS) return CurrentIdx;

	int32 NextIdx = CurrentIdx;
	for (int32 i = 0; i < MaxCount; i++)
	{
		if (bForward)
			NextIdx = (NextIdx + 1) % MaxCount;
		else
			NextIdx = (NextIdx - 1 + MaxCount) % MaxCount;

		if (!MyGS->OccupiedIndices.Contains(NextIdx))
		{
			return NextIdx;
		}
	}

	return CurrentIdx;
}
void UFPLobbyWidget::HandleNextClicked()
{
	int32 NewIndex = GetAvailableIndex(CurrentPreviewIndex, true);
	CurrentPreviewIndex = NewIndex;

	if (ImgCharPreview && CharacterImages.IsValidIndex(CurrentPreviewIndex))
	{
		ImgCharPreview->SetBrushFromTexture(CharacterImages[CurrentPreviewIndex]);
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (AFPPlayerController* FPPC = Cast<AFPPlayerController>(PC))
	{
		FPPC->ServerRequestCharacterUpdate(CurrentPreviewIndex);
	}
}

void UFPLobbyWidget::HandlePrevClicked()
{
	int32 NewIndex = GetAvailableIndex(CurrentPreviewIndex, false);
	CurrentPreviewIndex = NewIndex;

	if (ImgCharPreview && CharacterImages.IsValidIndex(CurrentPreviewIndex))
	{
		ImgCharPreview->SetBrushFromTexture(CharacterImages[CurrentPreviewIndex]);
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (AFPPlayerController* FPPC = Cast<AFPPlayerController>(PC))
	{
		FPPC->ServerRequestCharacterUpdate(CurrentPreviewIndex);
	}
}
void UFPLobbyWidget::NativeDestruct()
{
	Super::NativeDestruct();
	//타이머정리(메모리 누수 방지)
	GetWorld()->GetTimerManager().ClearTimer(PlayerCheckTimerHandle);

	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}
void UFPLobbyWidget::OnPlayerJoined(const FString& PlayerName, int32 CharacterIndex)
{
	// 중앙 슬롯(또는 캐릭터 프리뷰) 추가
	if (PlayerSlotWidgetClass && PlayerSpawnBox)
	{
		UFPPlayerSlotWidget* NewSlot = CreateWidget<UFPPlayerSlotWidget>(this, PlayerSlotWidgetClass);
		if (NewSlot)
		{
			NewSlot->SetPlayerName(PlayerName);

			if (CharacterImages.IsValidIndex(CharacterIndex))
			{
				NewSlot->SetCharacterImage(CharacterImages[CharacterIndex]);
			}

			UHorizontalBoxSlot* HSlot = PlayerSpawnBox->AddChildToHorizontalBox(NewSlot);
			if (HSlot)
			{
				FSlateChildSize FillSize;
				FillSize.SizeRule = ESlateSizeRule::Fill;
				FillSize.Value = 1.0f;
				HSlot->SetSize(FillSize);
				HSlot->SetPadding(FMargin(5.0f));
			}

			SpawnSlotMap.Add(PlayerName, NewSlot);
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
	if (!GS || !RedTeamListBox || !BlueTeamListBox) return;
	TArray<AFPPlayerState*> BlueTeamPlayers;
	TArray<AFPPlayerState*> RedTeamPlayers;
	RedTeamListBox->ClearChildren();
	BlueTeamListBox->ClearChildren();
	ListEntryMap.Empty();
	//현재 접속중인 플레이어 이름
	TArray<FString> CurrentPlayerNames;
	for (APlayerState* PS : GS->PlayerArray)
	{
		if (!PS || PS->IsABot()) continue;

		AFPPlayerState* FPS = Cast<AFPPlayerState>(PS);

		FString Name = (FPS && !FPS->CustomPlayerName.IsEmpty())
			? FPS->CustomPlayerName
			: PS->GetPlayerName();
		UE_LOG(LogTemp, Warning, TEXT("%s / TeamID = %d / Ready = %s / CharacterIndex = %d"),
			*Name,
			FPS ? (int32)FPS->TeamID : -1,
			(FPS && FPS->bIsReady) ? TEXT("TRUE") : TEXT("FALSE"),
			FPS ? FPS->CharacterIndex : -1);
		CurrentPlayerNames.Add(Name);

		if (FPS)
		{
			if (FPS->TeamID == EFPTeamID::TeamBlue)
			{
				BlueTeamPlayers.Add(FPS);
			}
			else if (FPS->TeamID == EFPTeamID::TeamRed)
			{
				RedTeamPlayers.Add(FPS);
			}
		}

		if (PlayerListEntryWidgetClass)
		{
			UFPPlayerListEntryWidget* NewEntry = CreateWidget<UFPPlayerListEntryWidget>(this, PlayerListEntryWidgetClass);
			if (NewEntry)
			{
				UTexture2D* SelectedIcon = DefaultIcon;
				if (FPS && CharacterImages.IsValidIndex(FPS->CharacterIndex))
				{
					SelectedIcon = CharacterImages[FPS->CharacterIndex];
				}

				NewEntry->SetPlayerInfo(Name, FPS ? FPS->bIsReady : false, SelectedIcon);

				UVerticalBox* TargetBox = nullptr;

				if (FPS)
				{
					if (FPS->TeamID == EFPTeamID::TeamRed)
					{
						TargetBox = RedTeamListBox;
					}
					else if (FPS->TeamID == EFPTeamID::TeamBlue)
					{
						TargetBox = BlueTeamListBox;
					}
				}

				if (TargetBox)
				{
					UVerticalBoxSlot* VSlot = TargetBox->AddChildToVerticalBox(NewEntry);
					if (VSlot)
					{
						VSlot->SetPadding(FMargin(0.0f, 5.0f));
					}
				}

				ListEntryMap.Add(Name, NewEntry);
			}
		}

	}
	UpdateCenterSlots(BlueTeamPlayers, RedTeamPlayers);

	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (LocalPC)
	{
		AFPPlayerState* LocalPS = LocalPC->GetPlayerState<AFPPlayerState>();
		if (LocalPS)
		{
			CurrentPreviewIndex = LocalPS->CharacterIndex;

			if (ImgCharPreview && CharacterImages.IsValidIndex(CurrentPreviewIndex))
			{
				ImgCharPreview->SetBrushFromTexture(CharacterImages[CurrentPreviewIndex]);
			}
		}
	}
	//캐시 업데이트
	CaChedPlayerNames = CurrentPlayerNames;
}
void UFPLobbyWidget::UpdateCenterSlots(const TArray<AFPPlayerState*>& BlueTeamPlayers, const TArray<AFPPlayerState*>& RedTeamPlayers)
{
	if (CenterSlot_Blue_0) CenterSlot_Blue_0->SetEmpty();
	if (CenterSlot_Blue_1) CenterSlot_Blue_1->SetEmpty();
	if (CenterSlot_Blue_2) CenterSlot_Blue_2->SetEmpty();
	if (CenterSlot_Red_0) CenterSlot_Red_0->SetEmpty();
	if (CenterSlot_Red_1) CenterSlot_Red_1->SetEmpty();
	if (CenterSlot_Red_2) CenterSlot_Red_2->SetEmpty();

	auto SetSlotFromPlayer = [this](UFPLobbyCenterSlotWidget* SlotWidget, AFPPlayerState* PS)
		{
			if (!SlotWidget || !PS)
			{
				return;
			}

			UTexture2D* Icon = DefaultIcon;
			if (CharacterImages.IsValidIndex(PS->CharacterIndex))
			{
				Icon = CharacterImages[PS->CharacterIndex];
			}

			const FString DisplayName = !PS->CustomPlayerName.IsEmpty()
				? PS->CustomPlayerName
				: PS->GetPlayerName();

			SlotWidget->SetPlayerInfo(
				DisplayName,
				PS->bIsReady,
				Icon,
				PS->TeamID
			);
		};

	if (BlueTeamPlayers.Num() > 0) SetSlotFromPlayer(CenterSlot_Blue_0, BlueTeamPlayers[0]);
	if (BlueTeamPlayers.Num() > 1) SetSlotFromPlayer(CenterSlot_Blue_1, BlueTeamPlayers[1]);
	if (BlueTeamPlayers.Num() > 2) SetSlotFromPlayer(CenterSlot_Blue_2, BlueTeamPlayers[2]);

	if (RedTeamPlayers.Num() > 0) SetSlotFromPlayer(CenterSlot_Red_0, RedTeamPlayers[0]);
	if (RedTeamPlayers.Num() > 1) SetSlotFromPlayer(CenterSlot_Red_1, RedTeamPlayers[1]);
	if (RedTeamPlayers.Num() > 2) SetSlotFromPlayer(CenterSlot_Red_2, RedTeamPlayers[2]);
}
