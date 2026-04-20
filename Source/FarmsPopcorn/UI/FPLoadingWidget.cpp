#include "FPLoadingWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Player/FPIntroController.h"

void UFPLoadingWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    UE_LOG(LogTemp, Warning, TEXT("=== LoadingWidget Construct ==="));
    
    // BackgroundImage 표시
    if (BackgroundImage && BackgroundImageTexture)
    {
        BackgroundImage->SetVisibility(ESlateVisibility::Visible);
        BackgroundImage->SetBrushFromTexture(BackgroundImageTexture);
        UE_LOG(LogTemp, Warning, TEXT("✓ BackgroundImage displayed"));
    }
    
    // LogoImage 표시
    if (LogoImage && LogoImageTexture)
    {
        LogoImage->SetVisibility(ESlateVisibility::Visible);
        LogoImage->SetBrushFromTexture(LogoImageTexture);
        UE_LOG(LogTemp, Warning, TEXT("✓ LogoImage displayed"));
    }
    
    // TitleLine 표시
    if (TitleLine && TitleLineImageTexture)
    {
        TitleLine->SetVisibility(ESlateVisibility::Visible);
        TitleLine->SetBrushFromTexture(TitleLineImageTexture);
        UE_LOG(LogTemp, Warning, TEXT("✓ TitleLine displayed"));
    }
    
    // LoadingSpinner 표시 및 초기화
    if (LoadingSpinner && LoadingSpinnerTexture)
    {
        LoadingSpinner->SetVisibility(ESlateVisibility::Visible);
        LoadingSpinner->SetBrushFromTexture(LoadingSpinnerTexture);
        LoadingSpinner->SetRenderTransformAngle(0.0f);
        UE_LOG(LogTemp, Warning, TEXT("✓ LoadingSpinner initialized"));
    }
    
    // LoadingText 초기화
    if (LoadingText)
    {
        LoadingText->SetText(FText::FromString(TEXT("로딩 중...")));
        UE_LOG(LogTemp, Warning, TEXT("✓ LoadingText initialized"));
    }
    
    LoadingTimer = 0.0f;
    CurrentRotation = 0.0f;
    bLoadingComplete = false;
}

void UFPLoadingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    if (bLoadingComplete)
    {
        return;
    }
    
    // 로딩 타이머 증가
    LoadingTimer += InDeltaTime;
    
    // 회전각 업데이트
    CurrentRotation += RotationSpeed * InDeltaTime;
    
    // 360도 이상이면 초기화 (무한 회전)
    if (CurrentRotation >= 360.0f)
    {
        CurrentRotation -= 360.0f;
    }
    
    // LoadingSpinner 회전
    if (LoadingSpinner)
    {
        LoadingSpinner->SetRenderTransformAngle(CurrentRotation);
    }
    
    // 진행률 계산
    float Progress = FMath::Clamp(LoadingTimer / LoadingDuration, 0.0f, 1.0f);
    
    // 로딩 완료
    if (LoadingTimer >= LoadingDuration)
    {
        bLoadingComplete = true;
        
        // 로딩 타입별 처리
        switch (CurrentLoadingType)
        {
            case ELoadingType::Intro:
                OnIntroLoadingComplete();
                break;
            case ELoadingType::RoundTransition:
                OnRoundTransitionComplete();
                break;
            case ELoadingType::Custom:
                OnCustomLoadingComplete();
                break;
        }
    }
}

void UFPLoadingWidget::StartLoading(ELoadingType LoadingType, float Duration)
{
    CurrentLoadingType = LoadingType;
    LoadingDuration = Duration;
    LoadingTimer = 0.0f;
    bLoadingComplete = false;
    
    UE_LOG(LogTemp, Warning, TEXT("⏳ Loading Started - Type: %d, Duration: %.1f sec"), 
        static_cast<int32>(LoadingType), Duration);
    
    // 기본 텍스트 설정
    if (LoadingText)
    {
        switch (LoadingType)
        {
            case ELoadingType::Intro:
                LoadingText->SetText(FText::FromString(TEXT("게임 시작 중...")));
                break;
            case ELoadingType::RoundTransition:
                LoadingText->SetText(FText::FromString(TEXT("다음 라운드 준비 중...")));
                break;
            case ELoadingType::Custom:
                LoadingText->SetText(FText::FromString(TEXT("로딩 중...")));
                break;
        }
    }
}

void UFPLoadingWidget::SetLoadingText(const FString& Text)
{
    if (LoadingText)
    {
        LoadingText->SetText(FText::FromString(Text));
        UE_LOG(LogTemp, Warning, TEXT("📝 Loading Text: %s"), *Text);
    }
}

void UFPLoadingWidget::SetLoadingProgress(float Progress)
{
    // 외부에서 진행률을 제어하고 싶을 때 사용
    LoadingTimer = Progress * LoadingDuration;
    LoadingTimer = FMath::Clamp(LoadingTimer, 0.0f, LoadingDuration);
}

void UFPLoadingWidget::CompleteLoading()
{
    // 외부에서 즉시 로딩 완료를 원할 때 사용
    if (!bLoadingComplete)
    {
        LoadingTimer = LoadingDuration;
        bLoadingComplete = true;
        
        switch (CurrentLoadingType)
        {
            case ELoadingType::Intro:
                OnIntroLoadingComplete();
                break;
            case ELoadingType::RoundTransition:
                OnRoundTransitionComplete();
                break;
            case ELoadingType::Custom:
                OnCustomLoadingComplete();
                break;
        }
    }
}

void UFPLoadingWidget::OnIntroLoadingComplete()
{
    UE_LOG(LogTemp, Warning, TEXT("✓ Intro Loading Complete - Moving to next level"));

    // 외부 주입 경로 우선 사용
    FString LevelPath = OverrideLevelPath;

    // 없으면 등록된 소프트 레퍼런스에서 추출
    if (LevelPath.IsEmpty() && !NextLevel.IsNull())
    {
        LevelPath = NextLevel.ToSoftObjectPath().GetLongPackageName();
    }

    if (LevelPath.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("OnIntroLoadingComplete: 이동할 레벨이 설정되지 않았습니다!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("→ ServerTravel: %s"), *LevelPath);

    if (UWorld* World = GetWorld())
    {
        if (World->GetNetMode() == NM_Client)
        {
            if (AFPIntroController* IntroPC = Cast<AFPIntroController>(GetOwningPlayer()))
            {
                IntroPC->Server_TravelToLevel(LevelPath);
            }
            return;
        }

        World->ServerTravel(LevelPath + TEXT("?listen"), true);
    }
}

void UFPLoadingWidget::OnRoundTransitionComplete()
{
    UE_LOG(LogTemp, Warning, TEXT("✓ Round Transition Complete"));
    
    // 이벤트 브로드캐스트 (GameMode에서 다음 라운드 시작)
    OnLoadingComplete.Broadcast();
    
    // 위젯 제거
    RemoveFromParent();
}

void UFPLoadingWidget::OnCustomLoadingComplete()
{
    UE_LOG(LogTemp, Warning, TEXT("✓ Custom Loading Complete"));
    
    // 이벤트 브로드캐스트
    OnLoadingComplete.Broadcast();
    
    // 위젯 제거
    RemoveFromParent();
}
