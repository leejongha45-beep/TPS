#pragma once
#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "TPSEnemyActorRefFragment.generated.h"

/**
 * 적 Actor 참조 Fragment
 * - FullActor LOD일 때 풀에서 꺼낸 Actor 1:1 매핑
 * - LODProcessor가 전환 시 저장/해제
 * - MovementProcessor가 유효하면 SetActorLocation 호출
 * - DeathProcessor가 유효하면 PlayDeath 호출
 * - None/ISM LOD에선 nullptr
 */
USTRUCT()
struct FTPSEnemyActorRefFragment : public FMassFragment
{
	GENERATED_BODY()

	/** 풀에서 꺼낸 Actor 약참조 */
	TWeakObjectPtr<AActor> ActorRef = nullptr;
};
