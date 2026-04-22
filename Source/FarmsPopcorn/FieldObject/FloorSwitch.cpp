// FloorSwitch.cpp

#include "FloorSwitch.h"
#include "MovingPlatform.h" // 발판 클래스 헤더 포함
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"

AFloorSwitch::AFloorSwitch()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    
    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = RootScene;

    SwitchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwitchMesh"));
    SwitchMesh->SetupAttachment(RootScene);

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetupAttachment(SwitchMesh);
    TriggerBox->SetBoxExtent(FVector(50.0f, 50.0f, 20.0f));
}

void AFloorSwitch::BeginPlay()
{
    Super::BeginPlay();
    if (!HasAuthority()) return;
    
    StartPos = SwitchMesh->GetRelativeLocation();
    TargetPos = StartPos - FVector(0.0f, 0.0f, SwitchLowerDepth);

    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AFloorSwitch::OnOverlapBegin);
    TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AFloorSwitch::OnOverlapEnd);
}

void AFloorSwitch::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!HasAuthority()) return;

    FVector CurrentPos = SwitchMesh->GetRelativeLocation();
    FVector Target = bIsPressed ? TargetPos : StartPos;
    FVector NewPos = FMath::VInterpTo(CurrentPos, Target, DeltaTime, LerpSpeed);

    SwitchMesh->SetRelativeLocation(NewPos);
    ReplicatedSwitchLocation = NewPos;
}

void AFloorSwitch::OnRep_SwitchLocation()
{
    SwitchMesh->SetRelativeLocation(ReplicatedSwitchLocation);
}

void AFloorSwitch::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this)
    {
        bIsPressed = true;

        // 연결된 모든 발판을 활성화합니다.
        for (AMovingPlatform* Platform : TargetPlatforms)
        {
            if (Platform)
            {
                Platform->SetPlatformActive(true);
            }
        }
    }
}

void AFloorSwitch::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor != this)
    {
        bIsPressed = false;

        // 연결된 모든 발판을 비활성화합니다.
        for (AMovingPlatform* Platform : TargetPlatforms)
        {
            if (Platform)
            {
                Platform->SetPlatformActive(false);
            }
        }
    }
}

void AFloorSwitch::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFloorSwitch, ReplicatedSwitchLocation);
}
