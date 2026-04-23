#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "FPLoadingWidget.generated.h"

UENUM(BlueprintType)
enum class ELoadingType : uint8
{
    Intro,           // 인트로 로딩 (자동 다음 레벨)
    RoundTransition, // 라운드 전환 (점수 표시)
    Custom           // 커스텀 (외부에서 제어)
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadingComplete);

UCLASS()
class FARMSPOPCORN_API UFPLoadingWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    
    // ===== 외부에서 호출 =====
    UFUNCTION(BlueprintCallable, Category = "Loading")
    void StartLoading(ELoadingType LoadingType, float Duration = 3.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Loading")
    void SetLoadingText(const FString& Text);
    
    UFUNCTION(BlueprintCallable, Category = "Loading")
    void SetLoadingProgress(float Progress);  // 0.0 ~ 1.0
    
    UFUNCTION(BlueprintCallable, Category = "Loading")
    void CompleteLoading();

    // 이벤트
    UPROPERTY(BlueprintAssignable, Category = "Loading")
    FOnLoadingComplete OnLoadingComplete;
protected:
    // UI 이미지들
    UPROPERTY(meta = (BindWidget))
    UImage* BackgroundImage = nullptr;
    
    UPROPERTY(meta = (BindWidget))
    UImage* LogoImage = nullptr;
    
    UPROPERTY(meta = (BindWidget))
    UImage* TitleLine = nullptr;
    
    UPROPERTY(meta = (BindWidget))
    UImage* LoadingSpinner = nullptr;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* LoadingText = nullptr;
    
    // 이미지 텍스처
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    UTexture2D* BackgroundImageTexture = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    UTexture2D* LogoImageTexture = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    UTexture2D* TitleLineImageTexture = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    UTexture2D* LoadingSpinnerTexture = nullptr;
    
    // 타이밍
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    float RotationSpeed = 360.0f;
    
   
public:   
    // 다음 레벨 (Intro 타입 전용) - 에디터에서 레벨 에셋을 직접 등록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    TSoftObjectPtr<UWorld> NextLevel;

    // GameMode에서 외부 주입용 (이 값이 있으면 NextLevel보다 우선)
    UPROPERTY(BlueprintReadWrite, Category = "Loading")
    FString OverrideLevelPath = TEXT("");    // 다음 레벨 (Intro 타입 전용)
    

private:
    ELoadingType CurrentLoadingType = ELoadingType::Custom;
    float LoadingTimer = 0.0f;
    float LoadingDuration = 3.0f;
    float CurrentRotation = 0.0f;
    bool bLoadingComplete = false;
    
    void OnIntroLoadingComplete();
    void OnRoundTransitionComplete();
    void OnCustomLoadingComplete();
};