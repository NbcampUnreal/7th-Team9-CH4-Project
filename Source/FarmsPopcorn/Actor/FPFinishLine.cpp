#include "FPFinishLine.h"
#include "Components/BoxComponent.h"
#include "Game/FPGameMode.h"
#include "Player/FPPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/FPPlayerCharacter.h"

AFPFinishLine::AFPFinishLine()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    RootComponent = CollisionBox;

    CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AFPFinishLine::OnOverlapBegin);
}

void AFPFinishLine::BeginPlay()
{
    Super::BeginPlay();
}

void AFPFinishLine::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;

    AFPPlayerCharacter* OverlappedChar = Cast<AFPPlayerCharacter>(OtherActor);
    if (!OverlappedChar)
    {
        // 2. 캐스팅 실패 이유 확인 (다른 컴포넌트나 액터일 경우)
        UE_LOG(LogTemp, Error, TEXT("FinishLine: Overlapped Actor is NOT AFPPlayerCharacter!"));
        return;
    }
    if (OverlappedChar)
    {
        AFPPlayerState* PS = OverlappedChar->GetPlayerState<AFPPlayerState>();
        if (PS)
        {
            EFPTeamID MyTeam = PS->GetTeamID();
            FString TeamName = (MyTeam == EFPTeamID::TeamRed) ? TEXT("RED") : TEXT("BLUE");

            AFPGameMode* GM = GetWorld()->GetAuthGameMode<AFPGameMode>();
            if (GM)
            {
                GM->AddScoreToTeam(MyTeam, 1);
                UE_LOG(LogTemp, Warning,
                       TEXT("플레이어의 팀 %s의 점수를 올랐습니다. RED : %d, BLUE : %d"),
                       *TeamName, GM->RedTeamScore, GM->BlueTeamScore);

                // 중복 처리 방지
                CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

                // 다음 레벨 이동 
                GetWorld()->GetTimerManager().SetTimer(
                    NextLevelHandle,
                    this,
                    &AFPFinishLine::NextMapTravel,
                    3.0f, // 3초 딜레이
                    false
                );
            }
        }
    }
}

void AFPFinishLine::NextMapTravel()
{
    if (NextLevel.IsNull()) // 할당 여부 체크
    {
        UE_LOG(LogTemp, Error, TEXT("ExecuteMapTravel: NextLevel이 블루프린트에서 설정되지 않았습니다!"));
        return;
    }

    // 소프트 포인터에서 전체 경로 문자열 추출 (가장 확실한 방법)
    FString LevelPath = NextLevel.ToSoftObjectPath().GetLongPackageName();
    
    if (LevelPath.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("ExecuteMapTravel: 유효한 레벨 경로를 찾을 수 없습니다!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("서버 트래블 시도 중: %s"), *LevelPath);
    
    GetWorld()->ServerTravel(LevelPath + TEXT("?listen"), true);

 
}