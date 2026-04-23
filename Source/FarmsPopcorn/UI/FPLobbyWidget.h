#pragma once
#include "Player/FPPlayerState.h"
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPLobbyWidget.generated.h"


class UButton;
class UTextBlock;
class UVerticalBox;
class UHorizontalBox;
class UTexture2D;
class UImage;
class UFPPlayerSlotWidget;
class UFPPlayerListEntryWidget;
class UFPLobbyCenterSlotWidget;
class AFPPlayerState;
class UFPCharacterDefinition;

UCLASS()
class FARMSPOPCORN_API UFPLobbyWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:
	//위젯 초기화
	virtual void NativeConstruct() override;
	//위젯 종료시 타이머 정리
	virtual void NativeDestruct() override;
	//플레이어 접속시 호출
	UFUNCTION(BlueprintCallable)
	void OnPlayerJoined(const FString& PlayerName, int32 CharacterIndex);
	//플레이어 퇴장시 호출
	UFUNCTION(BlueprintCallable)
	void OnPlayerLeft(const FString& PlayerName);
	//준비상태 변경시 호출
	UFUNCTION(BlueprintCallable)
	void OnPlayerReadyChanged(const FString& PlayerName, bool bNewReadyState);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
	int32 CurrentPreviewIndex = 0;


protected:
	//닉네임 표시
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* DisplayNickName;
	//레디 상태 표사
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* StatusText;
	//레디 버튼
	UPROPERTY(meta = (BindWidget))
	class UButton* ReadyButton;
	//레디 버튼 택스트
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ReadyButtonText;
	//중앙 캐릭터 표시
	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* PlayerSpawnBox;
	//플레이어 목록
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* BlueTeamListBox;
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* RedTeamListBox;
	UPROPERTY(meta = (BindWidget))
	class UImage* ImgCharPreview;

	UPROPERTY(meta = (BindWidget))
	class UFPLobbyCenterSlotWidget* CenterSlot_Blue_0;

	UPROPERTY(meta = (BindWidget))
	class UFPLobbyCenterSlotWidget* CenterSlot_Blue_1;

	UPROPERTY(meta = (BindWidget))
	class UFPLobbyCenterSlotWidget* CenterSlot_Blue_2;

	UPROPERTY(meta = (BindWidget))
	class UFPLobbyCenterSlotWidget* CenterSlot_Red_0;

	UPROPERTY(meta = (BindWidget))
	class UFPLobbyCenterSlotWidget* CenterSlot_Red_1;

	UPROPERTY(meta = (BindWidget))
	class UFPLobbyCenterSlotWidget* CenterSlot_Red_2;



	//슬롯 위젯
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UFPPlayerSlotWidget> PlayerSlotWidgetClass;
	//플레이어 목록전용 위젯
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UFPPlayerListEntryWidget> PlayerListEntryWidgetClass;
	//캐릭터 아이콘
	UPROPERTY(EditDefaultsOnly)
	TArray<class UTexture2D*> CharacterImages;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	class UTexture2D* DefaultIcon;
	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Prev;
	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Next;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	TArray<TObjectPtr<UFPCharacterDefinition>> CharacterDefinitions;


	//준비 버튼 클릭
	UFUNCTION()
	void OnReadyClicked();
	//준비 상태에 따라 UI 업데이트
	void UpdateReadyStatus(bool bNewReadyState);
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
	void OnClickNextCharacter();
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
	void OnClickPrevCharacter();
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	int32 GetAvailableIndex(int32 CurrentIdx, bool bForward);


	UFUNCTION()
	void HandleNextClicked();
	UFUNCTION()
	void HandlePrevClicked();

private:
	bool bIsReady = false; //현재 레디 상태
	//닉네임을 기준으로 슬롯 관리
	UPROPERTY()
	TMap<FString, class UFPPlayerSlotWidget*> SpawnSlotMap;
	//닉네임을 기준으로 목록관리
	UPROPERTY()
	TMap<FString, class UFPPlayerListEntryWidget*> ListEntryMap;
	//타이머
	FTimerHandle PlayerCheckTimerHandle;
	//플레이어 목록 변경시 감지
	TArray<FString> CaChedPlayerNames;
	//타이머에서 호출
	UFUNCTION()
	void CheckPlayerArray();
	void UpdateCenterSlots(const TArray<AFPPlayerState*>& BlueTeamPlayers, const TArray<AFPPlayerState*>& RedTeamPlayers);
};
