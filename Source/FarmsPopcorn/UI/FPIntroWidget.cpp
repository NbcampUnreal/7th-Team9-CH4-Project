#include "FPIntroWidget.h"
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
    if (MediaPlayer)
    {
        MediaPlayer->OnEndReached.RemoveDynamic(this, &UFPIntroWidget::OnMediaPlayerFinished);
        MediaPlayer->Close();
        MediaPlayer->Pause();
    }
    
    // 사운드 정지
    if (MediaSoundComponent)
    {
        MediaSoundComponent->SetVolumeMultiplier(0.0f);
        MediaSoundComponent->Stop();
        UE_LOG(LogTemp, Warning, TEXT("✓ MediaSoundComponent stopped"));
    }
    
    // 오버레이 정리
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
    
    // ⭐ 월드에 배치된 MediaSoundComponent 찾기 (첫 번째 실행 시만)
    if (!MediaSoundComponent)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            UMediaSoundComponent* SoundComp = Actor->FindComponentByClass<UMediaSoundComponent>();
            if (SoundComp)
            {
                MediaSoundComponent = SoundComp;
                UE_LOG(LogTemp, Warning, TEXT("✓ Found MediaSoundComponent in world actor: %s"), *Actor->GetName());
                break;
            }
        }
        
        if (!MediaSoundComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("⚠ MediaSoundComponent not found in world - sound may not work properly"));
            UE_LOG(LogTemp, Warning, TEXT("⚠ Make sure you have a Media Actor with MediaSoundComponent placed in the level"));
        }
    }
    
    // MediaSoundComponent 연결
    if (MediaSoundComponent)
    {
        MediaSoundComponent->SetMediaPlayer(MediaPlayer);
        MediaSoundComponent->SetVolumeMultiplier(1.0f);
        UE_LOG(LogTemp, Warning, TEXT("✓ MediaSoundComponent linked to MediaPlayer"));
    }
    
    // MediaPlayer 자체 사운드 활성화 (백업)
    MediaPlayer->SetNativeVolume(1.0f);
    
    // 영상 재생
    MediaPlayer->Play();
    UE_LOG(LogTemp, Warning, TEXT("▶ PLAYING VIDEO"));
    UE_LOG(LogTemp, Warning, TEXT("   - VideoPlayDuration: %.2f sec"), VideoPlayDuration);
    UE_LOG(LogTemp, Warning, TEXT("   - DissolveSpeed: %.2f sec"), DissolveSpeed);
    
    // 이벤트 바인딩 (중복 방지를 위해 먼저 제거)
    MediaPlayer->OnEndReached.RemoveDynamic(this, &UFPIntroWidget::OnMediaPlayerFinished);
    MediaPlayer->OnEndReached.AddDynamic(this, &UFPIntroWidget::OnMediaPlayerFinished);
}

void UFPIntroWidget::OnMediaPlayerFinished()
{
    if (bHasTransitioned)
    {
        return; // 중복 호출 방지
    }
    
    bHasTransitioned = true;
    UE_LOG(LogTemp, Warning, TEXT("📹 VIDEO FINISHED - Starting %.2f second fade out"), DissolveSpeed);
    
    CurrentPhase = EIntroPhase::Transition;
    PhaseTimer = 0.0f;
    
    // 영상 멈추기
    if (MediaPlayer)
    {
        MediaPlayer->Pause();
    }
    
    // 사운드 페이드 아웃
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