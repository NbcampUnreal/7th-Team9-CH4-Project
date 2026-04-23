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
        // 1. 데이터 테이블이 비어있지 않은지 검사
        if (!ItemDataTable)
        {
            UE_LOG(LogTemp, Warning, TEXT("ItemDataTable is Missing in FPItem!"));
            return;
        }

        // 2. 데이터 테이블의 모든 행(Row) 가져오기
        TArray<FItemData*> AllItems;
        ItemDataTable->GetAllRows<FItemData>(TEXT("ItemSpawnContext"), AllItems);

        // 3. 전체 가중치(SpawnWeight) 합산 구하기
        int32 TotalWeight = 0;
        for (FItemData* ItemInfo : AllItems)
        {
            TotalWeight += ItemInfo->SpawnWeight;
        }

        // 4. 1부터 총 가중치 사이에서 랜덤 숫자 뽑기
        int32 RandomWeight = FMath::RandRange(1, TotalWeight);
        int32 CurrentWeight = 0;
        EItemType SelectedItem = EItemType::None;

        // 5. 어떤 아이템이 당첨되었는지 확인 (룰렛 방식)
        for (FItemData* ItemInfo : AllItems)
        {
            CurrentWeight += ItemInfo->SpawnWeight;
            if (RandomWeight <= CurrentWeight)
            {
                SelectedItem = ItemInfo->ItemType;
                break; // 당첨! 루프 종료
            }
        }

        // 6. 플레이어에게 아이템 지급
        Player->PickupItem(SelectedItem);


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
