#pragma once

#include "CoreMinimal.h"
#include "RootYawOffsetMode.generated.h"

/**
 * 루트 Yaw 오프셋 처리 모드
 * - Lyra 스타일 Turn-In-Place 시스템에서 사용
 * - TPSPlayerCoreAnimInstance에서 상태에 따라 모드 전환
 */
UENUM(BlueprintType)
enum class ERootYawOffsetMode : uint8
{
	BlendOut    UMETA(DisplayName = "Blend Out"),   /** 오프셋을 점진적으로 0으로 복귀 */
	Hold        UMETA(DisplayName = "Hold"),        /** 현재 오프셋 값 유지 */
	Accumulate  UMETA(DisplayName = "Accumulate")   /** 회전 입력을 오프셋에 누적 */
};
