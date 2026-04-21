#include "FPIntroWidget.h"

#include "EngineUtils.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "FPLoadingWidget.h"

void UFPIntroWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 데디케이티드 서버로 자동 접속
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        //아래의 IP는 서버의 IP로, 서버가 바뀔 때 IP도 그에 맞춰서 변경 해줘야함.
        PC->ClientTravel(TEXT("220.77.247.47:7777"), ETravelType::TRAVEL_Absolute);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== IntroWidget Construct ==="));
    
    // MediaPlayer 확인
    if (!MediaPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ MediaPlayer is NULL!"));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("✓ MediaPlayer Found"));
    
    // MediaTexture 확인
    if (!MediaTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ MediaTexture is NULL!"));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("✓ MediaTexture Found"));
    
    // VideoMaterial 확인
    if (!VideoMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ VideoMaterial is NULL!"));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("✓ VideoMaterial Found"));
    
    // VideoImage에 Material 할당
    if (VideoImage)
    {
        VideoImage->SetBrushFromMaterial(VideoMaterial);
        VideoImage->SetVisibility(ESlateVisibility::Visible);
        UE_LOG(LogTemp, Warning, TEXT("✓ VideoImage connected to Material"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ VideoImage is NULL"));
        return;
    }
    
    // BackgroundOverlay 생성 (VideoImage 뒤에 배치)
    if (UCanvasPanel* Canvas = Cast<UCanvasPanel>(GetRootWidget()))
    {
        BackgroundOverlay = NewObject<UImage>(Canvas);
        
        // 배경을 검은색으로 설정
        FSlateBrush BackBrush;
        BackBrush.TintColor = FLinearColor::Black;
        BackgroundOverlay->SetBrush(BackBrush);
        BackgroundOverlay->SetRenderOpacity(1.0f);
        
        // Canvas에 추가
        UPanelSlot* PanelSlot = Canvas->AddChild(BackgroundOverlay);
        
        // Z-Order 설정 (가장 뒤)
        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(PanelSlot))
        {
            CanvasSlot->SetZOrder(-1);
            CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
            CanvasSlot->SetOffsets(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
            
            UE_LOG(LogTemp, Warning, TEXT("✓ BackgroundOverlay created"));
        }
    }
    
    // 영상 시작
    PlayVideoPhase();
}

void UFPIntroWidget::NativeDestruct()
{
    if (MediaPlayer && MediaPlayer->OnEndReached.IsBound())
    {
        MediaPlayer->OnEndReached.RemoveDynamic(this, &UFPIntroWidget::OnMediaPlayerFinished);
        MediaPlayer->Close();
    }
    
    if (MediaSoundComponent)
    {
        MediaSoundComponent->Stop();
    }
    
    if (BackgroundOverlay)
    {
        BackgroundOverlay->RemoveFromParent();
    }
    
    Super::NativeDestruct();
}

void UFPIntroWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    if (CurrentPhase == EIntroPhase::Video)
    {
        // 타이머 증가
        PhaseTimer += InDeltaTime;
        
        // VideoPlayDuration > 0일 때만 강제 전환 사용 (기본은 영상 끝 이벤트 기반)
        if (VideoPlayDuration > 0.0f && PhaseTimer >= VideoPlayDuration && !bHasTransitioned)
        {
            UE_LOG(LogTemp, Warning, TEXT("⏱ Time reached: %.2f >= %.2f - Starting Transition"), 
                PhaseTimer, VideoPlayDuration);
            OnMediaPlayerFinished();
        }
    }
    else if (CurrentPhase == EIntroPhase::Transition)
    {
        PlayTransitionPhase(InDeltaTime);
    }
}

void UFPIntroWidget::PlayVideoPhase()
{
    CurrentPhase = EIntroPhase::Video;
    PhaseTimer = 0.0f;
    bHasTransitioned = false;

    if (!MediaPlayer) 
    {
        UE_LOG(LogTemp, Error, TEXT("❌ MediaPlayer is NULL"));
        return;
    }
    
    // MediaSoundComponent 찾기
    if (!MediaSoundComponent)
    {
        if (UWorld* World = GetWorld())
        {
            for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
            {
                if (UMediaSoundComponent* SoundComp = ActorItr->FindComponentByClass<UMediaSoundComponent>())
                {
                    MediaSoundComponent = SoundComp;
                    UE_LOG(LogTemp, Warning, TEXT("✓ Found MediaSoundComponent"));
                    break;
                }
            }
        }
    }
    
    if (MediaSoundComponent)
    {
        MediaSoundComponent->SetMediaPlayer(MediaPlayer);
        MediaSoundComponent->SetVolumeMultiplier(1.0f);
    }
    
    MediaPlayer->SetNativeVolume(1.0f);
    MediaPlayer->Play();
    
    UE_LOG(LogTemp, Warning, TEXT("▶ PLAYING VIDEO (Duration: %.2fs)"), VideoPlayDuration);
    
    // ⭐ 이미 바인드된 경우를 대비해 먼저 제거
    if (MediaPlayer->OnEndReached.IsBound())
    {
        MediaPlayer->OnEndReached.RemoveDynamic(this, &UFPIntroWidget::OnMediaPlayerFinished);
    }
    
    // 바인딩
    MediaPlayer->OnEndReached.AddDynamic(this, &UFPIntroWidget::OnMediaPlayerFinished);
}

void UFPIntroWidget::OnMediaPlayerFinished()
{
    if (bHasTransitioned)
    {
        return;
    }
    
    bHasTransitioned = true;
    UE_LOG(LogTemp, Warning, TEXT("📹 VIDEO FINISHED - Fading out..."));
    
    CurrentPhase = EIntroPhase::Transition;
    PhaseTimer = 0.0f;
    
    if (MediaPlayer)
    {
        MediaPlayer->Pause();
    }
    
    if (MediaSoundComponent)
    {
        MediaSoundComponent->SetVolumeMultiplier(0.0f);
    }
}


void UFPIntroWidget::PlayTransitionPhase(float DeltaTime)
{
    PhaseTimer += DeltaTime;
    
    // DissolveSpeed 시간 동안 투명도 변경
    float DissolveAlpha = FMath::Clamp(
        PhaseTimer / DissolveSpeed, 
        0.0f, 
        1.0f
    );
    
    // VideoImage의 투명도 감소 (1.0 → 0.0)
    float VideoOpacity = 1.0f - DissolveAlpha;
    
    if (VideoImage)
    {
        // ⭐ Material Brush는 SetRenderOpacity 사용!
        VideoImage->SetRenderOpacity(VideoOpacity);
        
        UE_LOG(LogTemp, Warning, TEXT("🎬 Transition: %.1f%% | Opacity: %.3f"), 
            DissolveAlpha * 100.0f, VideoOpacity);
    }
    
    // 디졸브 완료
    if (DissolveAlpha >= 1.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("✓✓✓ TRANSITION COMPLETE ✓✓✓"));
        
        // VideoImage 숨기기
        if (VideoImage)
        {
            VideoImage->SetVisibility(ESlateVisibility::Collapsed);
        }
        
        // 다음 단계로 진행: 로딩 위젯 표시
        CurrentPhase = EIntroPhase::Loading;
        ShowLoadingWidget();
    }
}

void UFPIntroWidget::ShowLoadingWidget()
{
    
    // LoadingWidget 생성
    if (LoadingWidgetClass)
    {
        APlayerController* PC = GetOwningPlayer();
        if (!PC)
        {
            PC = GetWorld()->GetFirstPlayerController();
        }
        
        if (PC)
        {
            UFPLoadingWidget* LoadingWidget = CreateWidget<UFPLoadingWidget>(PC, LoadingWidgetClass);
            if (LoadingWidget)
            {
                // Intro 로딩 타입으로 시작 (3초 후 자동으로 다음 레벨)
                LoadingWidget->StartLoading(ELoadingType::Intro, 3.0f);
                LoadingWidget->AddToViewport(0);
                
                UE_LOG(LogTemp, Warning, TEXT("✓ LoadingWidget created and displayed"));
                
                // 현재 IntroWidget 제거
                RemoveFromParent();
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ LoadingWidgetClass is not assigned!"));
    }
}