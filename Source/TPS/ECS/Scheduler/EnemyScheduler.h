#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include <entt/entt.hpp>
#include "ECS/System/DamageSystem.h"

/**
 * 적 ECS 스케줄러 — FTickableGameObject 기반
 * - entt::registry 소유
 * - Phase별 시스템 실행 순서 제어
 *
 * [GameThread] Tick()
 *   Phase 0:     PushToPrev_RenderProxy
 *   Phase 1:     UObject 캐싱
 *   Phase 1.5:   LODSystem (AccumDT + bShouldTick 결정)
 *   Phase 2~4:   Damage → AI → Attack → Separation ∥ Death
 *   Phase 5+6:   Animation ∥ Movement (TaskGraph)
 *   ── Barrier ──
 *   Phase 7~8:   Visualization → Cleanup
 *   ++FrameCounter
 */
class FEnemyScheduler : public FTickableGameObject
{
public:
	FEnemyScheduler();
	virtual ~FEnemyScheduler() override;

	// ── FTickableGameObject ──
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return bIsActive; }

	/** 초기화 — 리소스 할당, 초기 상태 설정 */
	void Initialize();

	/** 해제 — 리소스 정리 */
	void Release();

	/** Registry 접근 (시스템에서 사용) */
	FORCEINLINE entt::registry& GetRegistry() { return Registry; }

	void SetAttackRange(float InRange) { AttackRange = InRange; }
	void SetHISM(class UHierarchicalInstancedStaticMeshComponent* InHISM) { HISMRef = InHISM; }

	FORCEINLINE TArray<entt::entity>& GetInstanceToEntity() { return InstanceToEntity; }

	/** 외부에서 데미지 이벤트 적재 (프로젝타일 OnHit → EnemyManagerSubsystem 경유) */
	FORCEINLINE void QueueDamage(int32 InstanceIndex, float Damage)
	{
		DamageQueue.Add({ InstanceIndex, Damage });
	}

protected:
	/** ECS Entity/Component 저장소 — 모든 시스템이 이 Registry를 통해 데이터 접근 */
	entt::registry Registry;

	/** HISM InstanceIndex → entt::entity 역방향 룩업 테이블 (O(1) swap 보정용)
	 *  - Spawn 시 Add, Cleanup 시 swap+Pop으로 HISM과 동기화 유지
	 *  - DamageSystem에서 HitResult → Entity 특정에 활용 */
	TArray<entt::entity> InstanceToEntity;

	/** 데미지 이벤트 큐 — 프로젝타일 OnHit에서 적재, DamageSystem에서 소비
	 *  GameThread 단일 접근 (OnHit/Tick 모두 GameThread) */
	TArray<FDamageEvent> DamageQueue;

	/** HISM 컴포넌트 참조 — Visualization/Cleanup에서 인스턴스 갱신/제거에 사용
	 *  비소유 참조 (AEnemyRenderActor가 소유, UEnemyManagerSubsystem이 수명 관리) */
	class UHierarchicalInstancedStaticMeshComponent* HISMRef = nullptr;

	/** AI 공격 판정 거리 — AISystem에서 AttackCooldown 진입 기준 */
	float AttackRange = 150.f;

	/** 스케줄러 활성 상태 — false면 IsTickable()이 false를 반환하여 Tick 중단 */
	uint8 bIsActive : 1 = false;

	/** LOD 틱 주기 계산용 프레임 카운터 — FrameOffset과 조합하여 엔티티별 틱 시점 분산 */
	uint32 FrameCounter = 0;
};