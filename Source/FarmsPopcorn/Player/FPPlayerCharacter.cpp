#include "Player/FPPlayerCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"   // DOREPLIFETIME 매크로

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
}

void AFPPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
    if (IsLocallyControlled() == false) return; // 로컬 캐릭터가 아니면 바로 리턴

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (IsValid(PC) == false) return; // checkf 대신 안전한 if 체크

    UEnhancedInputLocalPlayerSubsystem* EILPS =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>
        (PC->GetLocalPlayer());
    if (IsValid(EILPS) == false) return; // 여기도 동일하게

    EILPS->AddMappingContext(InputMappingContext, 0);// 에디터에서 설정한 입력 매핑(WASD 등)을 실제로 활성화
}

void AFPPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
}

// ---------------------------------------------------------
// 리플리케이션 등록 , 네트워크 동기화 등록 
// ---------------------------------------------------------
void AFPPlayerCharacter::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    //DOREPLIFETIME(APRPlayerCharacter, CurrentItem);// 아이템 동기화
    //DOREPLIFETIME(APRPlayerCharacter, CharacterIndex);// 캐릭터 선택 동기화
   // DOREPLIFETIME(APRPlayerCharacter, MaxJumpCount);// 점프 횟수 동기화
}
