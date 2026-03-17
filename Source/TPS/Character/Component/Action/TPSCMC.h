#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Utils/Interface/Data/Interpolable.h"
#include "Utils/TickFunctions/FInterpolateTickFunction.h"
#include "TPSCMC.generated.h"

/**
 * 커스텀 캐릭터 무브먼트 컴포넌트
 * - UCharacterMovementComponent의 public 변수를 캡슐화 (FORCEINLINE Setter)
 * - IInterpolable 구현 — 스프린트 속도 보간 (FInterpolateTickFunction)
 * - 걷기 ↔ 달리기 전환 시 부드러운 속도 보간
 */
UCLASS()
class TPS_API UTPSCMC : public UCharacterMovementComponent, public IInterpolable
{
	GENERATED_BODY()

public:
	UTPSCMC();

	/** 이동 방향으로 회전 여부 설정 */
	FORCEINLINE void SetOrientRotationToMovement(bool bInput) { bOrientRotationToMovement = bInput; }

	/** 회전 속도 설정 */
	FORCEINLINE void SetRotationRate(const FRotator& InputRate) { RotationRate = InputRate; }

	/** 최대 걷기 속도 설정 */
	FORCEINLINE void SetMaxWalkSpeed(float InputSpeed) { MaxWalkSpeed = InputSpeed; }

protected:
	virtual void RegisterComponentTickFunctions(bool bRegister) override;
	void InitTickFunctions();

#pragma region Sprint

public:
	/** 스프린트 속도 보간 시작 (목표 속도 설정) */
	void UpdateSprintSpeed(float TargetSpeed);

protected:
	/** 보간 전용 Tick 함수 (게임 스레드) */
	FInterpolateTickFunction InterpolateTickFunction;

	/** IInterpolable — 매 프레임 속도 보간 */
	virtual void Interpolate_Tick(float DeltaTime) override;
	void SetInterpolateTickEnabled(bool bEnabled);

	float InterpTargetSpeed = 0.f;

	/** 속도 보간 속도 (FInterpTo Rate) */
	UPROPERTY(EditDefaultsOnly, Category = "Rate")
	float SpeedInterpRate = 5.f;
#pragma endregion
};