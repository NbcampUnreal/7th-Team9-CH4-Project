#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "FPPlayerCharacter.generated.h"


class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None        UMETA(DisplayName = "None"),
	Fan         UMETA(DisplayName = "선풍기"),
	Magnet      UMETA(DisplayName = "자석"),
	WaterBalloon UMETA(DisplayName = "물풍선"),
	SweetPotato UMETA(DisplayName = "고구마"),   // 이중점프
};

UCLASS()
class FARMSPOPCORN_API AFPPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

#pragma region ACharacter Override

public:
	AFPPlayerCharacter();

	virtual void SetupPlayerInputComponent(
		UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

	// 착지 시 점프 카운트 초기화
	virtual void Landed(const FHitResult& Hit) override;

#pragma endregion

#pragma region Components (카메라/스프링암)

public:
	FORCEINLINE USpringArmComponent* GetSpringArm() const
	{
		return SpringArm;
	}
	FORCEINLINE UCameraComponent* GetCamera() const
	{
		return Camera;
	}

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "PR|Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "PR|Components")
	TObjectPtr<UCameraComponent> Camera;

#pragma endregion

public:	
	virtual void Tick(float DeltaTime) override;

	

#pragma region Input (Enhanced Input)
   
private:
    void HandleMoveInput(const FInputActionValue& InValue);
    void HandleLookInput(const FInputActionValue& InValue);
    void HandleJumpInput();       // 이중점프 처리
    void HandleItemInput();       // Z키 아이템 사용

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
        Category = "PR|Input")
    TObjectPtr<UInputMappingContext> InputMappingContext;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
        Category = "PR|Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
        Category = "PR|Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
        Category = "PR|Input")
    TObjectPtr<UInputAction> JumpAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
        Category = "PR|Input")
    TObjectPtr<UInputAction> ItemAction;    // Z키

#pragma endregion

    // =========================================================
#pragma region Jump (이중점프 - 고구마 아이템 연동)
// =========================================================
private:
    // 현재 점프 횟수
    int32 JumpCount = 0;

    // 최대 점프 횟수 (기본 1, 고구마 아이템 시 2)
    UPROPERTY(VisibleAnywhere, Replicated,
        Category = "PR|Jump")
    int32 MaxJumpCount = 1;

#pragma endregion

    // =========================================================
#pragma region Item (아이템 시스템)
// =========================================================
public:
    // 아이템 박스 먹을 때 호출
    UFUNCTION(BlueprintCallable)
    void PickupItem(EItemType NewItem);

    // 현재 보유 아이템 반환
    UFUNCTION(BlueprintPure)
    EItemType GetCurrentItem() const { return CurrentItem; }

private:
    // 아이템 사용 실제 처리 (서버에서 실행)
    UFUNCTION(Server, Reliable)
    void Server_UseItem();

    // 모든 클라이언트에 아이템 효과 알림
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayItemEffect(EItemType UsedItem);

    // 아이템별 효과 함수들
    void UseFan();          // 선풍기: 주변 적 날려버림
    void UseMagnet();       // 자석: 앞 적 끌어당김
    void UseWaterBalloon(); // 물풍선: 2초간 이동 불가
    void UseSweetPotato();  // 고구마: 이중점프 활성화

protected:
    // 현재 보유 아이템 (서버→클라 동기화)
    UPROPERTY(ReplicatedUsing = OnRep_CurrentItem,
        BlueprintReadOnly, Category = "PR|Item")
    EItemType CurrentItem = EItemType::None;

    UFUNCTION()
    void OnRep_CurrentItem();   // 아이템 UI 갱신 트리거

#pragma endregion

    // =========================================================
#pragma region Character Select (캐릭터 선택 - 11종)
// =========================================================
public:
    // 선택된 캐릭터 인덱스 설정 (0~10)
    UFUNCTION(Server, Reliable)
    void Server_SetCharacterIndex(int32 InIndex);

protected:
    // 캐릭터 인덱스 (서버→클라 동기화)
    UPROPERTY(ReplicatedUsing = OnRep_CharacterIndex,
        BlueprintReadOnly, Category = "PR|Character")
    int32 CharacterIndex = 0;

    UFUNCTION()
    void OnRep_CharacterIndex();    // 메시 교체 트리거

    // 에디터에서 11개 스켈레탈 메시 등록
    UPROPERTY(EditDefaultsOnly, Category = "PR|Character")
    TArray<TObjectPtr<USkeletalMesh>> CharacterMeshes;

#pragma endregion

    // =========================================================
#pragma region Network
// =========================================================
public:
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma endregion
};
