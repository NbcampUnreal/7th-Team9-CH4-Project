// Mine.cpp

#include "Mine.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AMine::AMine()
{
    PrimaryActorTick.bCanEverTick = false;

    MineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MineMesh"));
    RootComponent = MineMesh;

    // 접근 감지
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(150.0f); // 기본 감지 반경

    // 활성화 전 행동 X
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 변수 기본값
    bIsActive = false;
    ActivationDelay = 1.0f;
    ExplosionRadius = 300.0f;
    // 넉백 세기
    KnockbackForce = FVector(1200.0f, 1200.0f, 2000.0f);
}

void AMine::BeginPlay()
{
    Super::BeginPlay();

    GetWorldTimerManager().SetTimer(ActivationTimerHandle, this, &AMine::ActivateMine, ActivationDelay, false);
}

void AMine::ActivateMine()
{
    bIsActive = true;

    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AMine::OnCharacterDetected);
}

void AMine::OnCharacterDetected(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bIsActive && OtherActor && OtherActor != this)
    {
        // 닿은 액터가 캐릭터인지 확인
        if (OtherActor->IsA(ACharacter::StaticClass()))
        {
            Pop();
        }
    }
}

void AMine::Pop()
{
    if (!HasAuthority()) return;
    
    FVector PopLocation = GetActorLocation();

    // 감지 반경 안에 있는 액터를 찾음
    TArray<AActor*> OverlappingActors;
    DetectionSphere->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

    // 범위 내의 모든 캐릭터에게 넉백 적용
    for (AActor* Actor : OverlappingActors)
    {
        ACharacter* Victim = Cast<ACharacter>(Actor);
        if (Victim)
        {
            // 지뢰 중심에서 캐릭터 방향 벡터 계산
            FVector LaunchDir = Victim->GetActorLocation() - PopLocation;
            LaunchDir.Normalize();

            // z값의 최소치
            LaunchDir.Z = FMath::Max(LaunchDir.Z, 0.5f);

            // 방향 * (X,Y,Z 벡터) = 최종 속도
            FVector FinalVelocity = LaunchDir * KnockbackForce;

            // 캐릭터 발사
            Victim->LaunchCharacter(FinalVelocity, true, true);
        }
    }
    
    Multicast_Pop();
}

void AMine::Multicast_Pop_Implementation()
{
    // 폭발 후 삭제
    Destroy();
}
