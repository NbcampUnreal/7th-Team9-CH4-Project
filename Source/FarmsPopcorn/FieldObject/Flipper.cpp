#include "FieldObject/Flipper.h"
#include "GameFramework/Character.h"

AFlipper::AFlipper()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    FlipperMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlipperMesh"));
    FlipperMesh->SetupAttachment(SceneRoot);

    // 모서리 통과 방지를 위한 물리 설정
    FlipperMesh->SetCollisionProfileName(TEXT("BlockAll"));
    FlipperMesh->SetNotifyRigidBodyCollision(true);
    FlipperMesh->SetGenerateOverlapEvents(true);
    FlipperMesh->BodyInstance.bUseCCD = true;

    FlipperMesh->OnComponentHit.AddDynamic(this, &AFlipper::OnHit);
    FlipperMesh->OnComponentBeginOverlap.AddDynamic(this, &AFlipper::OnOverlapBegin);
}

void AFlipper::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    //서버만 실행하고, 클라이언트는 실행 안시킴
    if (!HasAuthority())
    {
        return;
    }

    ElapsedTime += DeltaTime;
    float CycleTime = FMath::Fmod(ElapsedTime, CycleInterval);
    float Alpha = 0.0f;

    if (CycleTime < OutSpeed) Alpha = CycleTime / OutSpeed;
    else if (CycleTime < OutSpeed + StayTime) Alpha = 1.0f;
    else if (CycleTime < OutSpeed + StayTime + InSpeed) Alpha = 1.0f - ((CycleTime - (OutSpeed + StayTime)) / InSpeed);

    SceneRoot->SetRelativeRotation(FRotator(0, 0, Alpha * MaxRotationAngle));
}

void AFlipper::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    LaunchTarget(OtherActor);
}

void AFlipper::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    LaunchTarget(OtherActor);
}

void AFlipper::LaunchTarget(AActor* OtherActor)
{
    if (!HasAuthority())
    {
        return;
    }
    
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character)
    {
        float CycleTime = FMath::Fmod(ElapsedTime, CycleInterval);

        // [중요] 발판이 올라오는 구간이거나 이미 다 올라와서 멈춘 직후까지 보정 범위 확대
        if (CycleTime > 0.0f && CycleTime <= OutSpeed + 0.1f)
        {
            FVector PivotPos = SceneRoot->GetComponentLocation();
            FVector CharacterPos = Character->GetActorLocation();

            // 방향: 발판 면의 수직 방향(Up)을 기본으로 하되, 캐릭터 위치로 꺾어줌
            FVector UpDir = FlipperMesh->GetUpVector();
            FVector SideDir = (CharacterPos - PivotPos).GetSafeNormal();

            FVector FinalDir = FMath::Lerp(UpDir, SideDir, 0.4f);
            FinalDir.Z += 0.3f; // 확실하게 띄우기 위해 Z값 상향
            FinalDir.Normalize();

            // 힘: 모서리일수록(축에서 멀수록) 훨씬 강하게 (최대 2.5배)
            float Distance = FVector::Dist(CharacterPos, PivotPos);
            float Multiplier = FMath::Clamp(Distance / 200.0f, 1.0f, 2.5f);

            Character->LaunchCharacter(FinalDir * BaseLaunchStrength * Multiplier, true, true);
        }
    }
}