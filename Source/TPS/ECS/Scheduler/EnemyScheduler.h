#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "ECS/Data/DamageEvent.h"
#include "ECS/Renderer/AEnemyRenderActor.h"
#include "ThirdParty/EnTT/include/entt/entity/registry.hpp"

/**
 * 적 ECS 스케줄러 — FTickableGameObject 기반
 * - entt::registry 소유
 * - Phase별 시스템 실행 순서 제어
 * - LOD별 HISM 배열 참조 (Near/Mid/Far)
 *
 * [GameThread] Tick()
 *   Phase 0:     PushToPrev_RenderProxy
 *   Phase 1:     UObject 캐싱
 *   Phase 1.5:   LODSystem (AccumDT + bShouldTick 결정)
 *   Phase 2~4:   Damage → AI → Attack → Separation ∥ Death
 *   Phase 5+6:   Animation ∥ Movement (TaskGraph)
 *   ── Barrier ──
 *   Phase 7:     LOD Transition (HISM간 인스턴스 이동)
 *   Phase 7.5:   Visualization
 *   Phase 8:     Cleanup
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

	/** 초기화 — 리소스 할당, 초기 상태 설정
	 * @param InWorld  Tick에서 사용할 World 참조 */
	void Initialize(class UWorld* InWorld);

	/** 해제 — 리소스 정리 */
	void Release();

	/** Registry 접근 (시스템에서 사용) */
	FORCEINLINE entt::registry& GetRegistry() { return Registry; }

	void SetAttackRange(float InRange) { AttackRange = InRange; }

	/** LOD별 HISM 참조 세팅 — 초기화 시 RenderActor로부터 */
	void SetHISMs(class UInstancedStaticMeshComponent* const* InHISMs, int32 Count);

	/** LOD별 InstanceToEntity 접근 */
	FORCEINLINE TArray<entt::entity>& GetInstanceToEntity(int32 LODIndex) { return InstanceToEntityPerLOD[LODIndex]; }

	/** LOD별 HISM 접근 */
	FORCEINLINE class UInstancedStaticMeshComponent* GetHISM(int32 LODIndex) const { return HISMRefs[LODIndex]; }

	/** Tick 진입 시 호출할 콜백 — Subsystem이 FlushSpawnQueue를 바인딩
	 *  Scheduler가 Subsystem 타입에 직접 의존하지 않도록 TFunction 사용 (D 원칙) */
	TFunction<void()> PreTickCallback;

	/** 엔티티 존재 여부 — FlushSpawnQueue에서 첫 스폰 시 true */
	uint8 bHasEntities : 1 = false;

	/** 외부에서 데미지 이벤트 적재 (프로젝타일 OnHit → EnemyManagerSubsystem 경유) */
	FORCEINLINE void QueueDamage(int32 InstanceIndex, uint8 LODLevel, float Damage)
	{
		DamageQueue.Add({ InstanceIndex, LODLevel, Damage });
	}

	/** HISM 컴포넌트 → LOD 인덱스 역조회 (프로젝타일 OnHit에서 사용) */
	int32 FindLODIndexByHISM(const class UInstancedStaticMeshComponent* InHISM) const;

protected:
	/** ECS Entity/Component 저장소 — 모든 시스템이 이 Registry를 통해 데이터 접근 */
	entt::registry Registry;

	/** LOD별 InstanceToEntity 역방향 룩업 테이블 [LODLevel][InstanceIndex] → Entity */
	TArray<entt::entity> InstanceToEntityPerLOD[HISM_LOD_COUNT];

	/** 데미지 이벤트 큐 — 프로젝타일 OnHit에서 적재, DamageSystem에서 소비
	 *  GameThread 단일 접근 (OnHit/Tick 모두 GameThread) */
	TArray<FDamageEvent> DamageQueue;

	/** LOD별 HISM 컴포넌트 참조 — 비소유 참조 */
	class UInstancedStaticMeshComponent* HISMRefs[HISM_LOD_COUNT] = {};

	/** AI 공격 판정 거리 — AISystem에서 AttackCooldown 진입 기준 */
	float AttackRange = 150.f;

	/** Tick에서 사용할 World — Subsystem이 Initialize 시 전달 */
	TWeakObjectPtr<class UWorld> CachedWorld;

	/** 스케줄러 활성 상태 — false면 IsTickable()이 false를 반환하여 Tick 중단 */
	uint8 bIsActive : 1 = false;

	/** LOD 틱 주기 계산용 프레임 카운터 — FrameOffset과 조합하여 엔티티별 틱 시점 분산 */
	uint32 FrameCounter = 0;
};
