#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Utils/Interface/Action/Moveable.h"
#include "Utils/Interface/Action/Sprintable.h"
#include "Utils/Interface/Action/Jumpable.h"
#include "Utils/Interface/Action/Interactable.h"
#include "TPSCharacterBase.generated.h"

/**
 * 모든 캐릭터의 공통 베이스
 * - 이동/스프린트/점프/상호작용 인터페이스 구현
 * - 공통 컴포넌트: State, Status, Footstep, CMC
 * - 자식: ATPSSoldierBase (전투), ATPSCivilianNPC (비전투)
 */
UCLASS(Abstract)
class TPS_API ATPSCharacterBase
	: public ACharacter, public IMoveable, public ISprintable, public IJumpable, public IInteractable
{
	GENERATED_BODY()

public:
	ATPSCharacterBase(const FObjectInitializer& ObjectInitializer);

	FORCEINLINE class UTPSPlayerStateComponent* GetStateComponent() const { return StateComponentInst; }
	FORCEINLINE class UTPSPlayerStatusComponent* GetStatusComponent() const { return StatusComponentInst; }
	FORCEINLINE class UTPSFootstepComponent* GetFootstepComponent() const { return FootstepComponentInst; }

protected:
	virtual void PostInitializeComponents() override;
	virtual void OnJumped_Implementation() override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	/** 컴포넌트 생성 — 자식에서 override 시 Super:: 호출 필수 */
	virtual void CreateDefaultComponents();

#pragma region Component
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Action")
	TObjectPtr<class UTPSCMC> CMCInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|State")
	TObjectPtr<class UTPSPlayerStateComponent> StateComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Status")
	TObjectPtr<class UTPSPlayerStatusComponent> StatusComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Data")
	TObjectPtr<class UTPSFootstepComponent> FootstepComponentInst;
#pragma endregion

#pragma region IMoveable
	virtual void StartMove() override;
	virtual void Move(const FVector2D& InputVector) override;
	virtual void StopMove() override;
#pragma endregion

#pragma region ISprintable
	virtual void StartSprint() override;
	virtual void StopSprint() override;
#pragma endregion

#pragma region IJumpable
	virtual void StartJump() override;
	virtual void StopJump() override;
#pragma endregion

#pragma region IInteractable
	virtual void Interact() override;
#pragma endregion
};
