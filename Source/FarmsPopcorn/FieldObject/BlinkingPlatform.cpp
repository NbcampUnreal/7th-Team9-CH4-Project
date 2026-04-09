#include "BlinkingPlatform.h"

ABlinkingPlatform::ABlinkingPlatform()
{
    PrimaryActorTick.bCanEverTick = true;

    PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
    RootComponent = PlatformMesh;
}

void ABlinkingPlatform::BeginPlay()
{
    Super::BeginPlay();
    UpdatePlatformState(); // 초기 상태 설정
}

void ABlinkingPlatform::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    Timer += DeltaTime;

    // 설정한 주기(3초)가 지나면 상태 반전
    if (Timer >= Interval)
    {
        bIsVisible = !bIsVisible;
        UpdatePlatformState();
        Timer = 0.0f; // 타이머 초기화
    }
}

void ABlinkingPlatform::UpdatePlatformState()
{
    if (bIsVisible)
    {
        // 보일 때: 메시 출력 ON, 충돌 ON
        PlatformMesh->SetHiddenInGame(false);
        PlatformMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    else
    {
        // 투명할 때: 메시 출력 OFF, 충돌 OFF
        PlatformMesh->SetHiddenInGame(true);
        PlatformMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}