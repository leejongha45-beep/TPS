#pragma once
#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "TPSPlayerLocationSharedFragment.generated.h"

/**
 * 플레이어 위치 SharedFragment
 * - GameThread의 PlayerLocationProcessor가 매 프레임 갱신
 * - WorkerThread의 AI/Movement Processor가 읽기 전용으로 참조
 * - SharedFragment = 같은 아키타입의 모든 Entity가 공유하는 단일 인스턴스
 */
USTRUCT()
struct FTPSPlayerLocationSharedFragment : public FMassSharedFragment
{
	GENERATED_BODY()

	/** 플레이어 월드 위치 (프레임당 1회 갱신) */
	FVector PlayerLocation = FVector::ZeroVector;
};
