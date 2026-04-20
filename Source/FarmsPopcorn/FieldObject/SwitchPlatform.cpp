#include "SwitchPlatform.h"
#include "Components/BoxComponent.h"

ASwitchPlatform::ASwitchPlatform()
{
    PrimaryActorTick.bCanEverTick = true;

    // 1. 루트 컴포넌트
    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = RootScene;

    // 2. 스위치 메시 (루트에 부착)
    SwitchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwitchMesh"));
    SwitchMesh->SetupAttachment(RootScene);

    // 3. 트리거 박스 (스위치에 부착하여 함께 움직이게 설정)
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetupAttachment(SwitchMesh);
    TriggerBox->SetBoxExtent(FVector(50.0f, 50.0f, 20.0f));

    // 4. 상승 발판 메시 (루트에 부착)
    TargetPlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetPlatformMesh"));
    TargetPlatformMesh->SetupAttachment(RootScene);
}

void ASwitchPlatform::BeginPlay()
{
    Super::BeginPlay();

    // 초기 위치 저장 및 목표 위치 계산 (로컬 좌표 기준)
    SwitchStartPos = SwitchMesh->GetRelativeLocation();
    SwitchTargetPos = SwitchStartPos - FVector(0.0f, 0.0f, SwitchLowerDepth);

    PlatformStartPos = TargetPlatformMesh->GetRelativeLocation();
    PlatformTargetPos = PlatformStartPos + FVector(0.0f, 0.0f, PlatformRiseHeight);

    // 오버랩 이벤트 연결
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ASwitchPlatform::OnOverlapBegin);
    TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ASwitchPlatform::OnOverlapEnd);
}

void ASwitchPlatform::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // --- 스위치 이동 처리 ---
    FVector CurrentSwitchPos = SwitchMesh->GetRelativeLocation();
    FVector TargetS = bIsPressed ? SwitchTargetPos : SwitchStartPos;
    // VInterpTo를 사용하여 부드러운 이동 구현
    SwitchMesh->SetRelativeLocation(FMath::VInterpTo(CurrentSwitchPos, TargetS, DeltaTime, LerpSpeed));

    // --- 발판 이동 처리 ---
    FVector CurrentPlatformPos = TargetPlatformMesh->GetRelativeLocation();
    FVector TargetP = bIsPressed ? PlatformTargetPos : PlatformStartPos;
    TargetPlatformMesh->SetRelativeLocation(FMath::VInterpTo(CurrentPlatformPos, TargetP, DeltaTime, LerpSpeed));
}

void ASwitchPlatform::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this)
    {
        bIsPressed = true;
    }
}

void ASwitchPlatform::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor != this)
    {
        bIsPressed = false;
    }
}