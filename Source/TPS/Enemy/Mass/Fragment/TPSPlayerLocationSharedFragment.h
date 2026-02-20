#pragma once
#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "TPSPlayerLocationSharedFragment.generated.h"

/**
 * 플레이어/기지/타겟 위치 SharedFragment
 * - GameThread의 PlayerLocationProcessor가 매 프레임 갱신
 * - WorkerThread의 AI/Movement Processor가 읽기 전용으로 참조
 * - SharedFragment = 같은 아키타입의 모든 Entity가 공유하는 단일 인스턴스
 * - TargetLocations: ITargetable 구현 액터 위치 (고정 크기 — WorkerThread 메모리 할당 회피)
 */
USTRUCT()
struct FTPSPlayerLocationSharedFragment : public FMassSharedFragment
{
	GENERATED_BODY()

	/** 플레이어 월드 위치 (프레임당 1회 갱신) */
	FVector PlayerLocation = FVector::ZeroVector;

	/** 아군 기지 위치 (기본 진격 목표) */
	FVector AllyBaseLocation = FVector::ZeroVector;

	/** ITargetable 구현 액터들의 위치 배열 (고정 크기) */
	static constexpr int32 MaxTargets = 256;
	FVector TargetLocations[MaxTargets];
	int32 TargetCount = 0;

	/** 마지막 타겟 배열 갱신 시간 */
	double LastTargetUpdateTime = 0.0;

	/** 타겟 배열 갱신 주기 (초) — 매 프레임 순회 방지 */
	static constexpr double TargetUpdateInterval = 0.1; // 100ms (10Hz)
};
