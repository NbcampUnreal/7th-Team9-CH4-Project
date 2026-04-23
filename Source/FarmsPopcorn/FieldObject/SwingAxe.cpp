#include "FieldObject/SwingAxe.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

ASwingAxe::ASwingAxe()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);
    
    RunningTime = 0.0f;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    AxeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AxeMesh"));
    AxeMesh->SetupAttachment(SceneRoot);

    // [중요] 겹침(Overlap) 설정
    AxeMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    AxeMesh->SetGenerateOverlapEvents(true);

    // 이미지에 있던 OnOverlap 바인딩
    AxeMesh->OnComponentBeginOverlap.AddDynamic(this, &ASwingAxe::OnOverlap);
}

void ASwingAxe::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!HasAuthority())
    {
        return;
    }

    // 이미지에 있던 RunningTime 활용 로직
    RunningTime += DeltaTime;
    float NewRotation = FMath::Sin(RunningTime * SwingSpeed) * MaxSwingAngle;
    SceneRoot->SetRelativeRotation(FRotator(NewRotation, 0, 0));
}

void ASwingAxe::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority())
    {
        return;
    }
    
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character)
    {
        // 1. 방향: 도끼 중심(SceneRoot)에서 캐릭터 방향으로 밀어내기
        FVector LaunchDir = Character->GetActorLocation() - GetActorLocation();
        LaunchDir.Normalize();

        // 2. 수직 보정: 바닥으로 박히지 않게 살짝 위로 띄움
        LaunchDir.Z = 0.35f;
        LaunchDir.Normalize();

        // 3. 날리기 실행 (핵심: true, true 설정)
        // bXYOverride와 bZOverride를 true로 주어야 캐릭터의 기존 속도가 중첩되지 않습니다.
        Character->LaunchCharacter(LaunchDir * LaunchStrength, true, true);
    }
}