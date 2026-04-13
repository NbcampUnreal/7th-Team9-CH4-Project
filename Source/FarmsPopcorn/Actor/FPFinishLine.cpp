// Fill out your copyright notice in the Description page of Project Settings.


#include "FPFinishLine.h"
#include "Components/BoxComponent.h"
#include "Game/FPGameMode.h"
#include "Game/FPGameState.h"
#include "GameFramework/Character.h"
#include "Player/FPPlayerState.h"

// Sets default values
AFPFinishLine::AFPFinishLine()
{
	PrimaryActorTick.bCanEverTick = false;

	// 박스 컴포넌트 설정
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;

	// 오버랩 이벤트 연결
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AFPFinishLine::OnOverlapBegin);
}


// Called when the game starts or when spawned
void AFPFinishLine::BeginPlay()
{
	Super::BeginPlay();
	
}

void AFPFinishLine::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
	
	ACharacter* OverlappedChar = Cast<ACharacter>(OtherActor);
	if (OverlappedChar)
	{
		//  캐릭터의 PlayerState에서 팀 정보 가져오기 
		AFPPlayerState* PS = OverlappedChar->GetPlayerState<AFPPlayerState>();
		if (PS)
		{
			EFPTeamID MyTeam = PS->GetTeamID(); // PlayerState에 정의해둔 Getter
			FString TeamName = (MyTeam == EFPTeamID::TeamRed) ? TEXT("RED") : TEXT("BLUE");
			// GameMode에 접근하여 점수 추가
			AFPGameMode* GM = GetWorld()->GetAuthGameMode<AFPGameMode>();
			if (GM)
			{
				GM->AddScoreToTeam(MyTeam, 1); // 10점 추가
				UE_LOG(LogTemp, Error, TEXT("플레이어의 팀 %s의 점수를 %d점 올라갔습니다. RED : %d ,Blue : %d"),
				       *TeamName,1,
				       GM->RedTeamScore, 
				       GM->BlueTeamScore);
				// 중복 처리 방지 트리거 비활성화
				CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}
}



