#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utils/UENUM/State.h"
#include "TPSPlayerStateComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(StateLog, Log, All);

/**
 * 플레이어 상태 관리 컴포넌트
 * - EActionState 비트플래그로 복수 상태 동시 관리
 * - Add/Remove로 상태 추가/제거, Has로 상태 조회
 * - AnimInstance(NativeUpdateAnimation)에서 매 프레임 GetCurrentState로 캐싱
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TPS_API UTPSPlayerStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** 상태 플래그 추가 (OR 연산) */
	void AddState(EActionState InState);

	/** 상태 플래그 제거 (AND NOT 연산) */
	void RemoveState(EActionState InState);

	/** 모든 상태 초기화 */
	void ClearState();

	/** 특정 상태 플래그 포함 여부 확인 */
	FORCEINLINE bool HasState(EActionState InState) const { return EnumHasAnyFlags(CurrentState, InState); }

	/** 현재 전체 상태 비트마스크 반환 */
	FORCEINLINE EActionState GetCurrentState() const { return CurrentState; }

protected:
	/** 현재 활성 상태 비트마스크 */
	EActionState CurrentState = EActionState::None;
};