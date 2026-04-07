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
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
    GetCharacterMovement()->MaxWalkSpeed = 600.f; // 걷기 최대 속도
    GetCharacterMovement()->JumpZVelocity = 600.f;// 점프 위로 튀어오르는 힘
    GetCharacterMovement()->AirControl = 0.3f;// 공중에서 방향 조작 가능한 정도 (0=불가, 1=완전자유)

    // 스프링암 (PDF: TargetArmLength = 400)
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->TargetArmLength = 400.f;// 카메라를 캐릭터로부터 400cm 뒤로
    SpringArm->bUsePawnControlRotation = true;// 마우스로 카메라 회전 가능
    SpringArm->SetupAttachment(GetRootComponent());

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
}



// 입력 바인딩 (ItemAction 추가)
void AFPPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

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
    if (JumpCount < MaxJumpCount)// 아직 점프 가능 횟수가 남아있으면
    {
        JumpCount++;// 점프 횟수 1 증가
        ACharacter::Jump();// 실제 점프 실행
    }
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

void AFPPlayerCharacter::Server_UseItem_Implementation()
{
    switch (CurrentItem)
    {
    case EItemType::Fan:          UseFan();          break;
    case EItemType::Magnet:       UseMagnet();       break;
    case EItemType::WaterBalloon: UseWaterBalloon(); break;
    case EItemType::SweetPotato:  UseSweetPotato();  break;
    default: break;
    }

    // 사용 후 아이템 소비
    CurrentItem = EItemType::None;
    OnRep_CurrentItem();  // UI 갱신
}


void AFPPlayerCharacter::UseFan()
{
    // TODO: 주변 일정 범위 적 캐릭터에게 LaunchCharacter() 적용
    Multicast_PlayItemEffect(EItemType::Fan);
}

void AFPPlayerCharacter::UseMagnet()
{
    // TODO: 앞 방향 일정 거리 적 캐릭터 위치를 내 위치 앞으로 이동
    Multicast_PlayItemEffect(EItemType::Magnet);
}

void AFPPlayerCharacter::UseWaterBalloon()
{
    // TODO: 타겟 캐릭터 MovementMode를 2초간 Disabled 처리
    Multicast_PlayItemEffect(EItemType::WaterBalloon);
}

void AFPPlayerCharacter::UseSweetPotato()
{
    // 고구마: 이중점프 활성화
    MaxJumpCount = 2;
    Multicast_PlayItemEffect(EItemType::SweetPotato);
}

void AFPPlayerCharacter::Multicast_PlayItemEffect_Implementation(
    EItemType UsedItem)
{
    // TODO: 아이템별 이펙트/사운드/애니메이션 몽타주 재생
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
    // TODO: 아이템 HUD 아이콘 갱신 (UMG 연동)
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
