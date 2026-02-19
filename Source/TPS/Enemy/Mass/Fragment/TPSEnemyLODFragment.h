#pragma once
#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Utils/UENUM/EnemyLODLevel.h"
#include "TPSEnemyLODFragment.generated.h"

/**
 * 적 LOD Fragment
 * - LODProcessor가 거리 계산 → LOD 전환 판정
 * - FullActor: Actor 풀에서 꺼내 활성화
 * - ISM: ISMSubsystem에 인스턴스 추가
 * - None: 렌더링 없음, 데이터만 갱신
 */
USTRUCT()
struct FTPSEnemyLODFragment : public FMassFragment
{
	GENERATED_BODY()

	/** 현재 LOD 단계 */
	EEnemyLODLevel LODLevel = EEnemyLODLevel::None;

	/** 플레이어까지 거리 (캐싱) */
	float DistanceToPlayer = 0.f;

	/** ISM 인스턴스 인덱스 (-1 = 미사용) */
	int32 ISMInstanceIndex = -1;
};
