#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include <entt/entt.hpp>

/**
 * 적 ECS 스케줄러 — FTickableGameObject 기반
 * - entt::registry 소유
 * - Phase별 시스템 실행 순서 제어
 *
 * [GameThread] Tick()
 *   → PushToPrev_RenderProxy → Phase_AI → Phase_Death → Phase_Animation
 *   → Phase_Movement → Phase_Visualization → Phase_Cleanup
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

protected:
	/** ECS Entity/Component 저장소 — 모든 시스템이 이 Registry를 통해 데이터 접근 */
	entt::registry Registry;

	/** HISM InstanceIndex → entt::entity 역방향 룩업 테이블 (O(1) swap 보정용)
	 *  - Spawn 시 Add, Cleanup 시 swap+Pop으로 HISM과 동기화 유지
	 *  - 향후 DamageSystem에서 HitResult → Entity 특정에도 활용 */
	TArray<entt::entity> InstanceToEntity;

	/** HISM 컴포넌트 참조 — Visualization/Cleanup에서 인스턴스 갱신/제거에 사용
	 *  비소유 참조 (AEnemyRenderActor가 소유, UEnemyManagerSubsystem이 수명 관리) */
	class UHierarchicalInstancedStaticMeshComponent* HISMRef = nullptr;

	/** AI 공격 판정 거리 — AISystem에서 AttackReady/Attacking 전환 기준 */
	float AttackRange = 150.f;

	/** 스케줄러 활성 상태 — false면 IsTickable()이 false를 반환하여 Tick 중단 */
	uint8 bIsActive : 1 = false;
};