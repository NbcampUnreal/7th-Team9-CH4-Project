#include "UI/FPScoreResultWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Engine/Engine.h"

void UFPScoreResultWidget::NativeConstruct()
{
	Super::NativeConstruct();
	//UI화면에서는 마우스 커서가 보이도록 설정
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputMode);
	}

	if (Btn_BackToLobby)
	{
		Btn_BackToLobby->OnClicked.AddDynamic(this, &UFPScoreResultWidget::OnBackToLobbyClicked); // 로비 이동 버튼
	}
	if (Btn_PlayAgain)
	{
		Btn_PlayAgain->OnClicked.AddDynamic(this, &UFPScoreResultWidget::OnPlayAgainClicked); // 다시 시작 버튼
	}
}

void UFPScoreResultWidget::SetGameResultData(const FFPGameResultData& InResultData)
{
	CurrentResultData = InResultData;
	//목표점수 설정
	TargetBlueScore = (float)InResultData.BlueTeamTotalScore;
	TargetRedScore = (float)InResultData.RedTeamTotalScore;
	//0점부터 천천히 올라감
	DisplayRedScore = 0.f; 
	DisplayBlueScore = 0.f;
	UpdataUI();
	//승패 텍스트 및 플레이어 리스트
	UpdateAllResults(CurrentResultData.bBlueTeamWin);
	UpdatePlayerList(VBox_BluePlayerList, CurrentResultData.BlueTeamPlayers);
	UpdatePlayerList(VBox_RedPlayerList, CurrentResultData.RedTeamPlayers);
}

void UFPScoreResultWidget::OnBackToLobbyClicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("로비로 돌아가기 버튼 클릭됨")); //로비 구현전 임시 확인버튼
	}
	APlayerController* OwningPlayer = GetOwningPlayer();
	if (OwningPlayer)
	{
		OwningPlayer->ClientTravel("경로", TRAVEL_Absolute); // 로비 경로 입력
	}
}

void UFPScoreResultWidget::OnPlayAgainClicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("다시 하기 버튼 클릭됨")); //다시가히 구현전 임시 확인버튼
	}
	APlayerController* Owningplayer = GetOwningPlayer();
	if (Owningplayer && Owningplayer->GetWorld())
	{
		FString CurrentMapName = Owningplayer->GetWorld()->GetMapName();//맵의 이름을 가져옴
		CurrentMapName.RemoveFromStart(Owningplayer->GetWorld()->StreamingLevelsPrefix);//PIE에서는 제거
		Owningplayer->ClientTravel(CurrentMapName, TRAVEL_Absolute);
	}
}

void UFPScoreResultWidget::UpdataUI()
{
	// 1000 단위에 ,(콤마) 찍음
	if (Text_BlueTeamScore) Text_BlueTeamScore->SetText(FText::AsNumber(CurrentResultData.BlueTeamTotalScore)); 
	if (Text_RedTeamScore) Text_RedTeamScore->SetText(FText::AsNumber(CurrentResultData.RedTeamTotalScore));

	UpdatePlayerList(VBox_BluePlayerList, CurrentResultData.BlueTeamPlayers);
	UpdatePlayerList(VBox_RedPlayerList, CurrentResultData.RedTeamPlayers);
}

void UFPScoreResultWidget::UpdatePlayerList(UVerticalBox* TargetVBox, const TArray<FFPPlayerResultEntry>& PlayerEntries)
{
	if (!TargetVBox || !PlayerEntryWidgetClass) return;
	TargetVBox->ClearChildren(); //리스트 초기화
	//개별위젯 생성 및 추가
	for (const FFPPlayerResultEntry& PlayerData : PlayerEntries)
	{
		UUserWidget* EntryWidget = CreateWidget<UUserWidget>(this, PlayerEntryWidgetClass);
		if (EntryWidget)
		{
			TargetVBox->AddChildToVerticalBox(EntryWidget);
		}
	}
}

void UFPScoreResultWidget::UpdateAllResults(bool bIsBlueWinner)
{
	//팀 결과
	SetIndividualResult(Text_BlueTeamResult, bIsBlueWinner);
	SetIndividualResult(Text_RedTeamResult, !bIsBlueWinner);
}
void UFPScoreResultWidget::SetIndividualResult(UTextBlock* TargetText, bool bIsWin)
{
	if (!TargetText) return;

	if (bIsWin)
	{
		TargetText->SetText(FText::FromString(TEXT("승리!")));  //승리 표기 
		TargetText->SetColorAndOpacity(FLinearColor(1.0f, 0.8f, 0.0f));
	}
	else
	{
		TargetText->SetText(FText::FromString(TEXT("패배..."))); //패배표기
		TargetText->SetColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f));
	}
}

void UFPScoreResultWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	//점수가 목표점수에 도달하지않르면 도달할때까지 가속,감속
	if (!FMath::IsNearlyEqual(DisplayBlueScore, TargetBlueScore, 0.1f))
	{
		DisplayBlueScore = FMath::FInterpTo(DisplayBlueScore, TargetBlueScore, InDeltaTime, LerpSpeed);
		if (Text_BlueTeamScore)
		{
			Text_BlueTeamScore->SetText(FText::AsNumber(FMath::RoundToInt(DisplayBlueScore)));
		}
	}
	if (!FMath::IsNearlyEqual(DisplayRedScore, TargetRedScore, 0.1f))
	{
		DisplayRedScore = FMath::FInterpTo(DisplayRedScore, TargetRedScore, InDeltaTime, LerpSpeed);
		if (Text_RedTeamScore)
		{
			Text_RedTeamScore->SetText(FText::AsNumber(FMath::RoundToInt(DisplayRedScore)));
		}
	}
}

