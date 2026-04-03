#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPScoreResultWidget.generated.h"

class UTextBlock;
class UImage;
class UVerticalBox;
class UButton;

USTRUCT(BlueprintType)
struct FFPPlayerResultEntry  //경기 결과 데이터
{
	GENERATED_BODY()

	FFPPlayerResultEntry() : PlayerName(TEXT("")), PlayerIcon(nullptr), Score(0)
	{}//초기화

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	FString PlayerName;//플레이어 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	UTexture2D* PlayerIcon;//플레이어 캐릭터의 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Score; //점수


};
USTRUCT(BlueprintType)
struct FFPGameResultData  //게임 전체 결과 데이터(두팀 전부)
{
		GENERATED_BODY()
		FFPGameResultData() : BlueTeamTotalScore(0), RedTeamTotalScore(0), bBlueTeamWin(false)
		{ }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 BlueTeamTotalScore; //블루팀 전체 점수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 RedTeamTotalScore;  //레드팀 전체 점수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bBlueTeamWin; //블루팀 승리 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FFPPlayerResultEntry> BlueTeamPlayers; //블루팀 플레이어 리스트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FFPPlayerResultEntry> RedTeamPlayers; //레드팀 플레이어 리스트
};


UCLASS()
class FARMSPOPCORN_API UFPScoreResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override; //위젯 생성시 호출
	UFUNCTION(BlueprintCallable, Category = "Result")
	void SetGameResultData(const FFPGameResultData& InResultData);//UI갱신
protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_BlueTeamScore; //블루팀 점수
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* VBox_BluePlayerList; // 블루팀 플레이어 리스트

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_RedTeamScore; //레드팀 점수
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* VBox_RedPlayerList; // 레드팀 플레이어 리스트

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_BackToLobby; //로비로 이동
	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_PlayAgain; //다시 시작

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_BlueTeamResult; //블루팀 승리/패배 문구
	UPROPERTY(meta= (BindWidget))
	class UTextBlock* Text_RedTeamResult; //레드팀 승리/패배 문구

	void UpdateAllResults(bool bIsBlueWinner);//승패 결과를 갱신

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Result")
	TSubclassOf<UUserWidget> PlayerEntryWidgetClass;//플레이어 리스트에 개별표기
private:
	UFUNCTION()
	void OnBackToLobbyClicked(); //로비버튼 클릭
	UFUNCTION()
	void OnPlayAgainClicked(); //다시 시작 버튼

	void UpdataUI(); //UI 갱신

	void UpdatePlayerList(UVerticalBox* TargetVBox, const TArray<FFPPlayerResultEntry>& PlayerEntries); //플레이어리스트를 VerticalBox로 생성

	void SetIndividualResult(UTextBlock* TargetText, bool bIsWin); //각 팀별 승패에 따라 설정되는 함수

	FFPGameResultData CurrentResultData; //결과 데이터

protected:
	//점수 0부터 천천히 올라가는 함수
	float DisplayBlueScore = 0.f;
	float TargetBlueScore = 0.f;
	float DisplayRedScore = 0.f;
	float TargetRedScore = 0.f;

	UPROPERTY(EditAnywhere, Category = "UI Effect")
	float LerpSpeed = 2.0f; //숫자 올라가는 속도

	virtual void NativeTick(const FGeometry&MyGeometry, float InDeltaTime) override; //매 프레임 숫자 호출
};
