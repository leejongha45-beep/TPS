#pragma once

#include "CoreMinimal.h"
#include "State.generated.h"

/**
 * 캐릭터 행동 상태 비트플래그
 * - 복수 상태 동시 활성 가능 (예: Moving | Aiming | Firing)
 * - TPSPlayerStateComponent에서 Add/Remove/Has로 관리
 * - AnimInstance에서 캐싱하여 스레드 안전하게 읽기
 */
UENUM(Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EActionState : uint16
{
	None        = 0       UMETA(Hidden),
	Idle        = 1 << 0, /** 정지 상태 */
	Moving      = 1 << 1, /** 이동 중 (WASD 입력) */
	Jumping     = 1 << 2, /** 점프 상승 중 */
	Falling     = 1 << 3, /** 낙하 중 */
	Sprinting   = 1 << 4, /** 달리기 (Shift) */
	Aiming      = 1 << 5, /** 조준 (RMB) */
	Equipping   = 1 << 6, /** 장착/해제 몽타주 진행 중 */
	Interacting = 1 << 7, /** 상호작용 중 */
	Firing      = 1 << 8, /** 사격 중 (LMB) */
};
ENUM_CLASS_FLAGS(EActionState);
