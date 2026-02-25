// WaveTypes.h

#pragma once

#include "CoreMinimal.h"
#include "WaveTypes.generated.h"

/* 웨이브 서브시스템의 현재 진행 단계 */
UENUM(BlueprintType)
enum class EWavePhase : uint8
{
	Idle,            // 전투 시작 전
	Trickle,         // 트리클 스폰 진행 중
	BigWaveAlert,    // 빅웨이브 경고 (UI 연출용)
	BigWaveActive,   // 빅웨이브 그룹 스폰 중
};
