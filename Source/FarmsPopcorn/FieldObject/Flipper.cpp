#include "Flipper.h"
#include "GameFramework/Character.h"

AFlipper::AFlipper()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    FlipperMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlipperMesh"));
    FlipperMesh->SetupAttachment(SceneRoot);

    FlipperMesh->SetCollisionProfileName(TEXT("Trigger"));
    FlipperMesh->OnComponentBeginOverlap.AddDynamic(this, &AFlipper::OnOverlap);
}

void AFlipper::BeginPlay()
{
    Super::BeginPlay();
}

void AFlipper::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ElapsedTime += DeltaTime;
    float CycleTime = FMath::Fmod(ElapsedTime, CycleInterval);
    float Alpha = 0.0f;

    // 1. 튀어나오는 구간
    if (CycleTime < OutSpeed)
    {
        Alpha = CycleTime / OutSpeed;
    }
    // 2. 중간에 멈춰있는 구간 (OutSpeed ~ OutSpeed + StayTime)
    else if (CycleTime < (OutSpeed + StayTime))
    {
        Alpha = 1.0f; // 최대 회전 상태 유지
    }
    // 3. 다시 들어가는 구간 (OutSpeed + StayTime ~ OutSpeed + StayTime + InSpeed)
    else if (CycleTime < (OutSpeed + StayTime + InSpeed))
    {
        float InProgress = (CycleTime - (OutSpeed + StayTime)) / InSpeed;
        Alpha = 1.0f - InProgress;
    }
    // 4. 대기 구간
    else
    {
        Alpha = 0.0f;
    }

    Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

    // 피벗(Root) 기준 Roll 회전
    FRotator NewRot = FRotator::ZeroRotator;
    NewRot.Roll = Alpha * MaxRotationAngle;

    RootComponent->SetRelativeRotation(NewRot);
}

void AFlipper::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character)
    {
        FVector LaunchDir = FlipperMesh->GetForwardVector() + FVector(0.0f, 0.0f, 0.5f);
        Character->LaunchCharacter(LaunchDir.GetSafeNormal() * LaunchStrength, true, true);
    }
}