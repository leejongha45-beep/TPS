#pragma once
#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "TPSEnemyMovementFragment.generated.h"

/**
 * 적 이동 Fragment
 * - Mass Entity 데이터 레이어에서 위치/이동 관리
 * - MovementProcessor가 TargetLocation 방향으로 CurrentLocation 갱신
 * - LODProcessor가 CurrentLocation으로 플레이어 거리 계산
 * - FullActor 전환 시 Actor 위치와 양방향 동기화
 */
USTRUCT()
struct FTPSEnemyMovementFragment : public FMassFragment
{
	GENERATED_BODY()

	/** 현재 위치 */
	FVector CurrentLocation = FVector::ZeroVector;

	/** 추적 대상 위치 */
	FVector TargetLocation = FVector::ZeroVector;

	/** 이동 속도 (cm/s) */
	float MoveSpeed = 600.f;
};
