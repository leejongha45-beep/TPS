#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TPSPlayerController.generated.h"

/**
 * 플레이어 컨트롤러
 * - Enhanced Input으로 입력 수신
 * - TScriptInterface를 통해 Pawn(Character)에 액션 위임
 * - Controller → Interface → Player 계층 분리
 * - OnPossess에서 Pawn의 인터페이스를 캐싱
 */
UCLASS()
class TPS_API ATPSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATPSPlayerController();

protected:
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;

#pragma region Input
	/** 마우스 시선 처리 */
	void Look(const struct FInputActionValue& InputValue);

	/** 이동 입력 (Start/Tick/Stop) */
	void StartMoveInput(const struct FInputActionValue& InputValue);
	void MoveInput(const struct FInputActionValue& InputValue);
	void StopMoveInput(const struct FInputActionValue& InputValue);

	/** 달리기 입력 (Start/Stop) */
	void StartSprintInput(const struct FInputActionValue& InputValue);
	void StopSprintInput(const struct FInputActionValue& InputValue);

	/** 조준 입력 (Start/Stop) */
	void StartAimInput(const struct FInputActionValue& InputValue);
	void StopAimInput(const struct FInputActionValue& InputValue);

	/** 점프 입력 (Start/Stop) */
	void StartJumpInput(const struct FInputActionValue& InputValue);
	void StopJumpInput(const struct FInputActionValue& InputValue);

	/** 장착/해제 토글 입력 */
	void EquipInput(const struct FInputActionValue& InputValue);

	/** 상호작용 입력 */
	void InteractInput(const struct FInputActionValue& InputValue);

	/** 사격 입력 (Start/Stop) */
	void StartFireInput(const struct FInputActionValue& InputValue);
	void StopFireInput(const struct FInputActionValue& InputValue);

	/** 재장전 입력 */
	void ReloadInput(const struct FInputActionValue& InputValue);

	/** 미니맵 토글 입력 */
	void MinimapInput(const struct FInputActionValue& InputValue);

	/** Enhanced Input — 기본 매핑 컨텍스트 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> DefaultMappingContextAsset;

	/** IA — 마우스 시선 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> LookActionAsset;

	/** IA — WASD 이동 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> MoveActionAsset;

	/** IA — Shift 달리기 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> SprintActionAsset;

	/** IA — RMB 조준 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> AimActionAsset;

	/** IA — Space 점프 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> JumpActionAsset;

	/** IA — 장착/해제 토글 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> EquipActionAsset;

	/** IA — E 상호작용 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> InteractActionAsset;

	/** IA — LMB 사격 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> FireActionAsset;

	/** IA — R 재장전 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> ReloadActionAsset;

	/** IA — M 미니맵 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> MinimapActionAsset;

	/** 이동 인터페이스 — OnPossess에서 캐싱 */
	UPROPERTY()
	TScriptInterface<class IMoveable> MoveableInterface;

	/** 달리기 인터페이스 — OnPossess에서 캐싱 */
	UPROPERTY()
	TScriptInterface<class ISprintable> SprintableInterface;

	/** 조준 인터페이스 — OnPossess에서 캐싱 */
	UPROPERTY()
	TScriptInterface<class IAimable> AimableInterface;

	/** 점프 인터페이스 — OnPossess에서 캐싱 */
	UPROPERTY()
	TScriptInterface<class IJumpable> JumpableInterface;

	/** 장착 인터페이스 — OnPossess에서 캐싱 */
	UPROPERTY()
	TScriptInterface<class IEquippable> EquippableInterface;

	/** 상호작용 인터페이스 — OnPossess에서 캐싱 */
	UPROPERTY()
	TScriptInterface<class IInteractable> InteractableInterface;

	/** 사격 인터페이스 — OnPossess에서 캐싱 */
	UPROPERTY()
	TScriptInterface<class IFireable> FireableInterface;
#pragma endregion
};