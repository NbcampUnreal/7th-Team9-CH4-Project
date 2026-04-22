#include "Player/FPPlayerCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "FPPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h" // DOREPLIFETIME 매크로
#include "FPPlayerState.h" 
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Engine/LocalPlayer.h"  //추가사항

AFPPlayerCharacter::AFPPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    // 회전 설정 (카메라 기준 이동)
    bUseControllerRotationPitch = false;
    // 캐릭터가 카메라 방향으로 자동 회전하지 않음
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    // 대신 이동 방향으로 캐릭터가 자연스럽게 돌아봄
    // (WASD 누르면 그 방향 바라봄 - 일반적인 3인칭 방식)
    GetCharacterMovement()->RotationRate = FRotator(0.f, 180.f, 0.f);
    GetCharacterMovement()->MaxWalkSpeed = 600.f; // 걷기 최대 속도
    GetCharacterMovement()->JumpZVelocity = 600.f;// 점프 위로 튀어오르는 힘
    GetCharacterMovement()->AirControl = 0.3f;// 공중에서 방향 조작 가능한 정도 (0=불가, 1=완전자유)

    // 스프링암 (PDF: TargetArmLength = 400)
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->TargetArmLength = 800.f;// 카메라를 캐릭터로부터 400cm 뒤로
    SpringArm->bUsePawnControlRotation = true;// 마우스로 카메라 회전 가능
    SpringArm->SetupAttachment(GetRootComponent());

    // 3. 회전 상속 설정 (핵심!)
    SpringArm->bInheritPitch = false; // 마우스 위아래 이동에 카메라가 기울어지지 않게 고정
    SpringArm->bInheritYaw = true;   // 마우스 좌우 이동에 카메라가 회전하도록 허용
    SpringArm->bInheritRoll = false;

    // [추가] 카메라가 바닥이나 벽에 닿았을 때 갑자기 튀는 것을 방지
    SpringArm->ProbeSize = 15.f;                 // 충돌 감지 크기 조절
    SpringArm->bDoCollisionTest = false;          // 벽 뚫기 방지
    SpringArm->SetRelativeRotation(FRotator(-30.f, 0.f, 0.f));
    SpringArm->SetRelativeLocation(FVector(0.f, 0.f, 30.f));

   

    // 카메라
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->bUsePawnControlRotation = false;
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

    // 리플리케이션 활성화
    bReplicates = true;// 이 캐릭터를 네트워크로 동기화 (멀티플레이 필수)

    // 머리 위 감지 박스 생성
    HeadDetectBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HeadDetectBox"));
    HeadDetectBox->SetupAttachment(GetRootComponent());
    HeadDetectBox->SetRelativeLocation(FVector(0.f, 0.f, 90.f)); // 머리 위 위치
    HeadDetectBox->SetBoxExtent(FVector(50.f, 50.f, 10.f));       // 납작한 박스
    HeadDetectBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    // [추가] 아이템 비주얼 컴포넌트 생성
    ItemVisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemVisualMesh"));

    // 이전에 만든 StackSocket에 부착 (머리 위 위치)
    ItemVisualMesh->SetupAttachment(GetMesh(), TEXT("StackSocket"));

    // 기본적으로는 보이지 않게 설정
    ItemVisualMesh->SetVisibility(false);
    ItemVisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 아이템 외형이 물리 충돌을 일으키지 않게 함


    // 1. 나이아가라 컴포넌트 생성
    JumpSmokeComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("JumpSmokeComponent"));

    // 2. [핵심] 메시의 "StackSocket" 위치에 부착
    // 부모 컴포넌트인 GetMesh() 뒤에 소켓 이름을 적어줍니다.
    JumpSmokeComponent->SetupAttachment(GetMesh(), TEXT("StackSocket"));

    // 자동 재생 방지 (필요할 때만 Activate 하기 위해)
    JumpSmokeComponent->bAutoActivate = false;
}

void AFPPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 이벤트 바인딩 (로컬/서버 상관없이 모두 등록)
    if (IsValid(HeadDetectBox))
    {
        HeadDetectBox->OnComponentBeginOverlap.AddDynamic(
            this, &ThisClass::OnHeadBoxBeginOverlap);
        HeadDetectBox->OnComponentEndOverlap.AddDynamic(
            this, &ThisClass::OnHeadBoxEndOverlap);
    }

    if (IsLocallyControlled() == false) return; // 로컬 캐릭터가 아니면 바로 리턴

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (IsValid(PC) == false) return; // checkf 대신 안전한 if 체크

    UEnhancedInputLocalPlayerSubsystem* EILPS =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>
        (PC->GetLocalPlayer());
    if (IsValid(EILPS) == false) return; // 여기도 동일하게

    EILPS->AddMappingContext(InputMappingContext, 0);// 에디터에서 설정한 입력 매핑(WASD 등)을 실제로 활성화
    OnRep_CharacterIndex();

   // TryApplyInputMappingContext();   //추가사항
   // OnRep_CharacterIndex();  //추가사항
    SyncCharacterVisualFromPlayerState();
}

void AFPPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	TryApplyInputMappingContext();
	SyncCharacterVisualFromPlayerState();
}

void AFPPlayerCharacter::OnRep_Controller()
{
    Super::OnRep_Controller();
    TryApplyInputMappingContext();
    SyncCharacterVisualFromPlayerState();
}



// 입력 바인딩 (ItemAction 추가)
void AFPPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    TryApplyInputMappingContext(); //추가 사항

    UEnhancedInputComponent* EIC =
        CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

    // WASD 이동
    EIC->BindAction(MoveAction, ETriggerEvent::Triggered,
        this, &ThisClass::HandleMoveInput);

    // 마우스 시점
    EIC->BindAction(LookAction, ETriggerEvent::Triggered,
        this, &ThisClass::HandleLookInput);

    // 점프 (이중점프 처리를 위해 별도 함수 사용)
    EIC->BindAction(JumpAction, ETriggerEvent::Started,
        this, &ThisClass::HandleJumpInput);
    EIC->BindAction(JumpAction, ETriggerEvent::Completed,
        this, &ACharacter::StopJumping);

    // Z키 아이템
    EIC->BindAction(ItemAction, ETriggerEvent::Started,
        this, &ThisClass::HandleItemInput);
}

// WASD 이동 (PDF HandleMoveInput과 동일)
// ---------------------------------------------------------
void AFPPlayerCharacter::HandleMoveInput(
    const FInputActionValue& InValue)
{
    if (IsValid(Controller) == false)
    {
        UE_LOG(LogTemp, Error, TEXT("Controller is invalid."));
        return;
    }

    const FVector2D InMovementVector = InValue.Get<FVector2D>();

    const FRotator ControlRotation = Controller->GetControlRotation();
    const FRotator ControlYawRotation(0.f, ControlRotation.Yaw, 0.f);

    const FVector ForwardDirection =
        FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection =
        FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::Y);

    AddMovementInput(ForwardDirection, InMovementVector.X);
    AddMovementInput(RightDirection, InMovementVector.Y);
}

// ---------------------------------------------------------
// 마우스 시점 (PDF HandleLookInput과 동일)
// ---------------------------------------------------------
void AFPPlayerCharacter::HandleLookInput(
    const FInputActionValue& InValue)
{
    if (IsValid(Controller) == false)
    {
        UE_LOG(LogTemp, Error, TEXT("Controller is invalid."));
        return;
    }

    const FVector2D InLookVector = InValue.Get<FVector2D>();
    AddControllerYawInput(InLookVector.X);
    AddControllerPitchInput(InLookVector.Y);
}

// ---------------------------------------------------------
// 점프 (이중점프 지원)
// ---------------------------------------------------------
void AFPPlayerCharacter::HandleJumpInput()
{
    Jump();
}

void AFPPlayerCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);// 언리얼 기본 착지 처리 먼저 실행
    JumpCount = 0;  // 착지 시 점프 카운트 초기화
}

// ---------------------------------------------------------
// Z키 아이템 사용
// ---------------------------------------------------------
void AFPPlayerCharacter::HandleItemInput()
{
    if (CurrentItem == EItemType::None) return;

    // 클라이언트 → 서버 RPC
    Server_UseItem();
}

void AFPPlayerCharacter::TryApplyInputMappingContext()
{
    if (bInputMappingApplied || !IsLocallyControlled() || !InputMappingContext)   //추가사항
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (IsValid(PC) == false) return; // checkf 대신 안전한 if 체크

    ULocalPlayer* LocalPlayer = PC->GetLocalPlayer(); //추가 사항
    if (!IsValid(LocalPlayer)) //추가 사항
    {
        return;
    }

    // UEnhancedInputLocalPlayerSubsystem* EILPS =
    //    ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>
    //    (PC->GetLocalPlayer());
    // if (IsValid(EILPS) == false) return; // 여기도 동일하게
    // 아래함수로 변경

    UEnhancedInputLocalPlayerSubsystem* EILPS =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
    if (!IsValid(EILPS))
    {
        return;
    }

    // EILPS->AddMappingContext(InputMappingContext, 0);// 에디터에서 설정한 입력 매핑(WASD 등)을 실제로 활성화
    // OnRep_CharacterIndex(); 
    // 아래함수로 변경

    EILPS->RemoveMappingContext(InputMappingContext);
    EILPS->AddMappingContext(InputMappingContext, 0);
    bInputMappingApplied = true;
}

void AFPPlayerCharacter::Server_UseItem_Implementation()
{
    switch (CurrentItem)
    {
    case EItemType::Fan:          UseFan();          break;
    case EItemType::Magnet:       UseMagnet();       break;
    case EItemType::WaterBalloon: UseWaterBalloon(); break;
    case EItemType::WaterBalloonFreeze: UseWaterBalloonFreeze(); break;
    case EItemType::SweetPotato:  UseSweetPotato();  break;
    default: break;
    }

    // 사용 후 아이템 소비
    CurrentItem = EItemType::None;
    OnRep_CurrentItem();  // UI 갱신
}





void AFPPlayerCharacter::UseFan()
{
    // 선풍기: 주변 일정 범위 적 캐릭터에게 LaunchCharacter() 적용
    TArray<FOverlapResult> OverlapResults;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(500.f);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->OverlapMultiByChannel(OverlapResults, GetActorLocation(), FQuat::Identity, ECC_Pawn, Sphere, Params))
    {
        for (auto& Result : OverlapResults)
        {
            AFPPlayerCharacter* OtherChar = Cast<AFPPlayerCharacter>(Result.GetActor());
            if (!IsValid(OtherChar)) continue;

            // ✅ 팀 체크 — 상대팀에게만 적용
            AFPPlayerState* MyPS = GetPlayerState<AFPPlayerState>();
            AFPPlayerState* OtherPS = OtherChar->GetPlayerState<AFPPlayerState>();
            if (!IsValid(MyPS) || !IsValid(OtherPS)) continue;
            if (MyPS->TeamID == OtherPS->TeamID) continue; // 같은 팀이면 스킵

            FVector LaunchDir = OtherChar->GetActorLocation() - GetActorLocation();
            LaunchDir.Normalize();
            OtherChar->LaunchCharacter(LaunchDir * 1500.f + FVector(0, 0, 500.f), true, true);
        }
    }
    Multicast_PlayItemEffect(EItemType::Fan);
}

void AFPPlayerCharacter::UseMagnet()
{
    if (!HasAuthority()) return;

    // UseFan()과 동일한 범위 검색 로직 활용
    TArray<FOverlapResult> OverlapResults;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(1000.f); // 감지 반경
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->OverlapMultiByChannel(OverlapResults, GetActorLocation(), FQuat::Identity, ECC_Pawn, Sphere, Params))
    {
        for (auto& Result : OverlapResults)
        {
            AFPPlayerCharacter* OtherChar = Cast<AFPPlayerCharacter>(Result.GetActor());
            if (!IsValid(OtherChar)) continue;

            // 기존 팀 체크 로직 그대로 활용
            AFPPlayerState* MyPS = GetPlayerState<AFPPlayerState>();
            AFPPlayerState* OtherPS = OtherChar->GetPlayerState<AFPPlayerState>();
            if (MyPS && OtherPS && MyPS->TeamID != OtherPS->TeamID)
            {
                // 상대방을 내 위치 방향으로 끌어당김
                FVector PullDir = GetActorLocation() - OtherChar->GetActorLocation();
                PullDir.Normalize();
                OtherChar->LaunchCharacter(PullDir * 2000.f + FVector(0, 0, 300.f), true, true);

                Multicast_PlayItemEffect(EItemType::Magnet);
                break; // 한 명에게 적용되면 종료
            }
        }
    }
}

void AFPPlayerCharacter::UseWaterBalloon()
{

    if (!HasAuthority()) return;

    TArray<FOverlapResult> OverlapResults;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(800.f); // 감지 반경
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->OverlapMultiByChannel(OverlapResults, GetActorLocation(), FQuat::Identity, ECC_Pawn, Sphere, Params))
    {
        for (auto& Result : OverlapResults)
        {
            AFPPlayerCharacter* OtherChar = Cast<AFPPlayerCharacter>(Result.GetActor());
            if (!IsValid(OtherChar)) continue;

            AFPPlayerState* MyPS = GetPlayerState<AFPPlayerState>();
            AFPPlayerState* OtherPS = OtherChar->GetPlayerState<AFPPlayerState>();
            if (MyPS && OtherPS && MyPS->TeamID != OtherPS->TeamID)
            {
                // 상대방 속도를 0으로 변경 (가두기 효과)
                OtherChar->GetCharacterMovement()->MaxWalkSpeed = 0.f;
                OtherChar->Multicast_StartWaterBalloonFreeze();

                // 헤더에 이미 선언된 WaterBalloonFreezeTimerHandle 사용
                GetWorldTimerManager().SetTimer(WaterBalloonFreezeTimerHandle, [OtherChar]()
                    {
                        if (IsValid(OtherChar))
                        {
                            OtherChar->GetCharacterMovement()->MaxWalkSpeed = 600.f; // 복구
                            OtherChar->Multicast_StopWaterBalloonFreeze();
                        }
                    }, 10.0f, false);

                Multicast_PlayItemEffect(EItemType::WaterBalloon);
                break;
            }
        }
    }
}

void AFPPlayerCharacter::Multicast_StartWaterBalloonFreeze_Implementation()
{
    // WaterBalloonEffect 변수에 에셋이 할당되어 있으면 그걸 우선 사용
    // 없으면 ItemEffects Map에서 WaterBalloonFreeze 키로 찾음
    UNiagaraSystem* Effect = nullptr;

    if (IsValid(WaterBalloonEffect))
    {
        Effect = WaterBalloonEffect;
    }
    else if (ItemEffects.Contains(EItemType::WaterBalloonFreeze))
    {
        Effect = ItemEffects[EItemType::WaterBalloonFreeze];
    }

    if (!IsValid(Effect)) return;

    // 캐릭터 루트에 붙여서 생성 → 캐릭터랑 같이 이동
    UNiagaraFunctionLibrary::SpawnSystemAttached(
        Effect,
        GetRootComponent(),
        NAME_None,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        true  // 나이아가라 자체 수명으로 자동 소멸
    );
}

void AFPPlayerCharacter::Multicast_StopWaterBalloonFreeze_Implementation()
{
    // 나이아가라 에셋 자체에서 10초 수명을 설정해놨으니
    // 속도 복구만 알려주면 됨 — 이펙트는 알아서 꺼짐
    // 필요하면 여기서 추가 처리 가능
}

void AFPPlayerCharacter::UseWaterBalloonFreeze()
{
}

void AFPPlayerCharacter::UseSweetPotato()
{
    // 고구마: 이중점프 활성화
    JumpMaxCount = 2;
    //Multicast_PlayItemEffect(EItemType::SweetPotato);

    // (팁) 5초 뒤에 다시 1단 점프로 돌려놓기
    //FTimerHandle SweetPotatoTimer;
    GetWorldTimerManager().SetTimer(SweetPotatoTimerHandle, [this]()
        {
            JumpMaxCount = 1;
        }, 5.0f, false);
}

void AFPPlayerCharacter::Jump()
{
    // 만약 고구마를 먹어서 점프 횟수가 2인 상태이고, 
    // 지금 한 번 점프한 상태(공중)에서 다시 점프를 누른 거라면?
    if (JumpMaxCount == 2 && JumpCurrentCount == 1)
    {
        // 이때만 고구마 이펙트 실행!
        Multicast_PlayItemEffect(EItemType::SweetPotato);
    }

    // "원래 언리얼이 하던 점프 기능"은 그대로 수행해! 라는 뜻입니다.
    Super::Jump();
}

void AFPPlayerCharacter::Multicast_PlayItemEffect_Implementation(
    EItemType UsedItem)
{
    if (ItemEffects.Contains(UsedItem))
    {
        UNiagaraSystem* SelectedEffect = ItemEffects[UsedItem];
        if (SelectedEffect)
        {
            // SpawnSystemAttached를 사용하면 캐릭터 몸(Mesh)에 이펙트가 붙어서 따라다닙니다.
            UNiagaraFunctionLibrary::SpawnSystemAttached(
                SelectedEffect,
                GetMesh(),                // 부착할 컴포넌트
                TEXT("StackSocket"),           // 부착할 뼈(Bone) 이름 (없으면 NAME_None)
                FVector::ZeroVector,      // 상대 위치
                FRotator::ZeroRotator,    // 상대 회전
                EAttachLocation::SnapToTarget,
                true                      // 자동 파괴 활성화
            );
        }
    
    }
    // 2. 사운드 재생 추가 (모든 플레이어의 위치에서 소리 발생)
    if (ItemSounds.Contains(UsedItem) && ItemSounds[UsedItem])
    {
        UGameplayStatics::PlaySoundAtLocation(this, ItemSounds[UsedItem], GetActorLocation());
    }
}

// ---------------------------------------------------------
// 아이템 박스 습득
// ---------------------------------------------------------
void AFPPlayerCharacter::PickupItem(EItemType NewItem)
{
    if (!HasAuthority()) return;  // 서버에서만 처리

    CurrentItem = NewItem;
    OnRep_CurrentItem();
}

void AFPPlayerCharacter::OnRep_CurrentItem()
{
    // 1. 아이템이 없으면 숨기기
    if (CurrentItem == EItemType::None)
    {
        ItemVisualMesh->SetVisibility(false);
        return;
    }

    // 2. 아이템이 있다면 보이기
    ItemVisualMesh->SetVisibility(true);

    // 3. 현재 아이템 종류에 따라 메시 교체
    UStaticMesh* SelectedMesh = nullptr;

    switch (CurrentItem)
    {
    case EItemType::Fan:          SelectedMesh = Mesh_Fan;          break;
    case EItemType::Magnet:       SelectedMesh = Mesh_Magnet;       break;
    case EItemType::WaterBalloon: SelectedMesh = Mesh_WaterBalloon; break;
    case EItemType::SweetPotato:  SelectedMesh = Mesh_SweetPotato;  break;
    default: break;
    }

    if (SelectedMesh)
    {
        ItemVisualMesh->SetStaticMesh(SelectedMesh);
    }
}

void AFPPlayerCharacter::UseCurrentItem()// Z키와 연결된 함수
{
    switch (CurrentItem)
    {
    case EItemType::Fan:
        UseFan();
        break;

    case EItemType::SweetPotato:
        UseSweetPotato();
        break;

    default:
        break;
    }
}

// ---------------------------------------------------------
// 캐릭터 선택 (11종)
// ---------------------------------------------------------
void AFPPlayerCharacter::Server_SetCharacterIndex_Implementation(
    int32 InIndex)
{
    if (!CharacterMeshes.IsValidIndex(InIndex)) return;

    CharacterIndex = InIndex;
    OnRep_CharacterIndex();
}

void AFPPlayerCharacter::OnRep_PlayerState()
{
}

void AFPPlayerCharacter::OnRep_CharacterIndex()
{
    if (CharacterMeshes.IsValidIndex(CharacterIndex))
    {
        GetMesh()->SetSkeletalMesh(CharacterMeshes[CharacterIndex]);
    }

    // 메시가 바뀐 후, 해당 메시에 맞는 ABP를 강제로 다시 할당해야 애니메이션이 활성화됩니다.
    if (CharacterAnimClassArray.IsValidIndex(CharacterIndex))
    {
        GetMesh()->SetAnimInstanceClass(CharacterAnimClassArray[CharacterIndex]);
    }
}

void AFPPlayerCharacter::SyncCharacterVisualFromPlayerState()
{
    {
        if (AFPPlayerState* FPPS = GetPlayerState<AFPPlayerState>())
        {
            CharacterIndex = FPPS->CharacterIndex;
            OnRep_CharacterIndex();

            if (!FPPS->CustomPlayerName.IsEmpty())
            {
                CurrentName = FPPS->CustomPlayerName;
            }
        }
    }
}





void AFPPlayerCharacter::OnHeadBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;
    // [필수 추가] 닿은 물체가 '나 자신'이면 여기서 바로 중단합니다. (에러 원인 차단)
    if (OtherActor == nullptr || OtherActor == this) return;

    AFPPlayerCharacter* Rider =
        Cast<AFPPlayerCharacter>(OtherActor);
    if (!IsValid(Rider)) return;

    // 내가(Under) 이미 누군가(Rider)에게 붙어있는 상태라면, 그 사람을 내 머리에 올릴 수 없습니다.
    if (this->GetAttachParentActor() == Rider) return;
    // [필수 추가] 내 머리 위에 탄 사람이 이미 다른 곳에 매달려있다면 중단합니다.
    if (Rider->MountedCharacter != nullptr) return;

    // 내(Under) 머리 위에 이미 다른 사람이 타고 있다면 (배열 체크)
    // 한 명씩만 정직하게 쌓고 싶다면 아래 코드를 활성화하세요.
    // if (RiderCharacters.Num() > 0) return;
    Rider->Server_MountOn(this);
}

void AFPPlayerCharacter::OnHeadBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!HasAuthority()) return;

    AFPPlayerCharacter* Rider =
        Cast<AFPPlayerCharacter>(OtherActor);
    if (!IsValid(Rider)) return;

    Rider->Server_Dismount();
}

// 서버에서 캐릭터 부착(올라타기) 처리
void AFPPlayerCharacter::Server_MountOn_Implementation(AFPPlayerCharacter* UnderCharacter)
{
    // [필수 추가] 밑에 깔릴 대상이 없거나, 그게 나 자신이면 중단합니다.
    if (!IsValid(UnderCharacter) || UnderCharacter == this) return;


    // 팀 확인 (같은 팀끼리만 올라탈 수 있음)
    AFPPlayerState* MyPS = GetPlayerState<AFPPlayerState>();
    AFPPlayerState* UnderPS =
        UnderCharacter->GetPlayerState<AFPPlayerState>();
    if (!IsValid(MyPS) || !IsValid(UnderPS)) return;
    //if (MyPS->TeamIndex != UnderPS->TeamIndex) return;

    // [필수 추가] 튕김 방지를 위해 부착되는 순간 나의 충돌과 이동을 완전히 끕니다.
    //GetCharacterMovement()->SetMovementMode(MOVE_None);
   // GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);


    // 물리적으로 부착
    AttachToActor(UnderCharacter, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("StackSocket"));

    MountedCharacter = UnderCharacter;
    UnderCharacter->RiderCharacters.Add(this);
    UnderCharacter->GetCharacterMovement()->MaxWalkSpeed = 300.f;

    MoveIgnoreActorAdd(UnderCharacter);           // 나(위) → 아래 캐릭터 무시
    UnderCharacter->MoveIgnoreActorAdd(this);     // 아래 → 나(위) 무시

    // 캡슐 충돌 채널에서 Pawn 무시 (튕김 방지 핵심!)
    GetCapsuleComponent()->SetCollisionResponseToChannel(
        ECC_Pawn, ECR_Ignore);
}

// 서버에서 캐릭터 분리(내려오기) 처리
void AFPPlayerCharacter::Server_Dismount_Implementation()
{
    if (!IsValid(MountedCharacter)) return;

    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform); // 부착 해제

    MountedCharacter->RiderCharacters.Remove(this);// 목록에서 제거

    MoveIgnoreActorRemove(MountedCharacter);          // 나 → 아래 무시 해제
    MountedCharacter->MoveIgnoreActorRemove(this);    // 아래 → 나 무시 해제

    // 캡슐 충돌 원래대로 복구
    GetCapsuleComponent()->SetCollisionResponseToChannel(
        ECC_Pawn, ECR_Block);

    // 위에 아무도 없으면 아래 사람 속도 원래대로 복구
    if (MountedCharacter->RiderCharacters.IsEmpty())
    {
        MountedCharacter->GetCharacterMovement()
            ->MaxWalkSpeed = 600.f;
    }
    MountedCharacter = nullptr;
}

// ---------------------------------------------------------
// 리플리케이션 등록 , 네트워크 동기화 등록 
// ---------------------------------------------------------
void AFPPlayerCharacter::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AFPPlayerCharacter, CurrentItem);// 아이템 동기화
    DOREPLIFETIME(AFPPlayerCharacter, CharacterIndex);// 캐릭터 선택 동기화
    DOREPLIFETIME(AFPPlayerCharacter, MaxJumpCount);// 점프 횟수 동기화
    DOREPLIFETIME(AFPPlayerCharacter, RiderCharacters);
    DOREPLIFETIME(AFPPlayerCharacter, MountedCharacter);
}
