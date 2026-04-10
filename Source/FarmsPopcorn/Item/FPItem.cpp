// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/FPItem.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Player/FPPlayerCharacter.h"

AFPItem::AFPItem()
{
 	
	PrimaryActorTick.bCanEverTick = true;
    bReplicates = true; // 멀티플레이 동기화

    // 충돌 박스 설정
    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    RootComponent = CollisionBox;
    CollisionBox->SetBoxExtent(FVector(50.f, 50.f, 50.f));
    CollisionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    // 메시 설정
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(RootComponent);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 메시는 충돌 무시
}


void AFPItem::BeginPlay()
{
	Super::BeginPlay();
	
    // 서버에서만 충돌 감지 처리
    if (HasAuthority())
    {
        CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AFPItem::OnOverlapBegin);
    }
}

void AFPItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;

    AFPPlayerCharacter* Player = Cast<AFPPlayerCharacter>(OtherActor);

    // 플레이어가 살아있고, 이미 들고 있는 아이템이 없을 때만 획득
    if (Player && Player->GetCurrentItem() == EItemType::None)
    {
        // 랜덤으로 아이템 결정 (None 제외 1~4번 중 하나)
        // EItemType::Fan ~ EItemType::SweetPotato
        int32 RandomItemIdx = FMath::RandRange(1, 4);
        EItemType RandomItem = static_cast<EItemType>(RandomItemIdx);

        // 플레이어에게 아이템 지급
        Player->PickupItem(RandomItem);

        // 상자 비활성화 (마리오카트처럼 먹으면 사라짐)
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);

        // 일정 시간 후 재생성
        GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AFPItem::RespawnItem, RespawnTime, false);
    }
}

void AFPItem::RespawnItem()
{
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
}
