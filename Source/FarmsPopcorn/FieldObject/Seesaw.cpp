#include "FieldObject/SeeSaw.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"

ASeeSaw::ASeeSaw()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    PlankMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlankMesh"));
    PlankMesh->SetupAttachment(SceneRoot);
    PlankMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    DetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectionBox"));
    DetectionBox->SetupAttachment(PlankMesh);
    DetectionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void ASeeSaw::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TArray<AActor*> OverlappingActors;
    DetectionBox->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

    float CombinedForce = 0.0f;

    if (OverlappingActors.Num() > 0)
    {
        for (AActor* Actor : OverlappingActors)
        {
            ACharacter* Character = Cast<ACharacter>(Actor);
            if (!Character) continue;

            FVector LocalPos = PlankMesh->GetComponentTransform().InverseTransformPosition(Character->GetActorLocation());
            float DistanceFromCenter = FMath::Abs(LocalPos.X);

            if (DistanceFromCenter < DeadzoneRadius)
            {
                // [데드존에 있는 사람] 현재 기울기를 0으로 되돌리려는 복원력을 기여
                // 현재 기울기의 반대 방향으로 복원 속도만큼 힘을 보탬
                CombinedForce -= (CurrentTilt * RecoverySpeed);
            }
            else
            {
                // [바깥에 있는 사람] 자신의 위치만큼 무게를 누적시키려는 힘을 기여
                float DistanceRatio = -(LocalPos.X / 400.0f);
                CombinedForce += (DistanceRatio * WeightSensitivity);
            }
        }

        // 최종 합산된 힘을 CurrentTilt에 적용
        CurrentTilt += CombinedForce * DeltaTime;
    }
    else
    {
        // 아무도 없을 때는 기존처럼 순수 복원
        CurrentTilt = FMath::FInterpTo(CurrentTilt, 0.0f, DeltaTime, RecoverySpeed);
    }

    // 최종 적용
    CurrentTilt = FMath::Clamp(CurrentTilt, -1.0f, 1.0f);
    PlankMesh->SetRelativeRotation(FRotator(CurrentTilt * MaxTiltAngle, 0.0f, 0.0f));
}