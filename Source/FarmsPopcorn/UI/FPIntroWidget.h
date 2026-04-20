#pragma once

#pragma once
 
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "MediaSoundComponent.h"
#include "FPIntroWidget.generated.h"
 
class UMediaPlayer;
class UMediaTexture;
class UCanvasPanelSlot;
class UFPLoadingWidget;
 
UCLASS()
class FARMSPOPCORN_API UFPIntroWidget : public UUserWidget
{
    GENERATED_BODY()
 
public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
 
protected:
    // ===== 영상 관련 =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro|Video")
    UMediaPlayer* MediaPlayer = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro|Video")
    UMediaTexture* MediaTexture = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro|Video")
    UMaterialInterface* VideoMaterial = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro|Video")
    UMediaSoundComponent* MediaSoundComponent = nullptr;
    
    // ===== UI 위젯 =====
    UPROPERTY(meta = (BindWidget))
    UImage* VideoImage = nullptr;
    
    // ===== 로딩 위젯 클래스 ⭐ =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro|Loading")
    TSubclassOf<UFPLoadingWidget> LoadingWidgetClass;
    
    // ===== 타이밍 =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro|Timing")
    float VideoPlayDuration = 5.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro|Timing")
    float DissolveSpeed = 2.0f;
 
private:
    UPROPERTY()
    UImage* BackgroundOverlay = nullptr;
    
    enum class EIntroPhase
    {
        Video,
        Transition,
        Loading
    };
    
    EIntroPhase CurrentPhase = EIntroPhase::Video;
    float PhaseTimer = 0.0f;
    bool bHasTransitioned = false;
    
    void OnMediaPlayerFinished();
    void PlayVideoPhase();
    void PlayTransitionPhase(float DeltaTime);
    void ShowLoadingWidget();
};