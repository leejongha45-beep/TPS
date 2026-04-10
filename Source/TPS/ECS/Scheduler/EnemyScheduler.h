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
 * - 웨이포인트 체인 캐싱 (Rush 경로)
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

	void Initialize(class UWorld* InWorld);
	void Release();

	FORCEINLINE entt::registry& GetRegistry() { return Registry; }

	void SetAttackRange(float InRange) { AttackRange = InRange; }

	/** 웨이포인트 체인 수집 — Head부터 링크 순회하여 CachedWaypoints에 적재 */
	void CollectWaypoints(class UWorld* World);

	FORCEINLINE const TArray<FVector>& GetWaypoints() const { return CachedWaypoints; }
	FORCEINLINE float GetWaypointAcceptRadius() const { return WaypointAcceptRadius; }

	void SetHISMs(class UInstancedStaticMeshComponent* const* InHISMs, int32 Count);

	FORCEINLINE TArray<entt::entity>& GetInstanceToEntity(int32 LODIndex) { return InstanceToEntityPerLOD[LODIndex]; }
	FORCEINLINE class UInstancedStaticMeshComponent* GetHISM(int32 LODIndex) const { return HISMRefs[LODIndex]; }

	TFunction<void()> PreTickCallback;
	TFunction<void(int32)> PostTickKillCallback;
	TFunction<void(const TArray<FHitEffectRequest>&)> HitEffectCallback;

	uint8 bHasEntities : 1 = false;

	FORCEINLINE void QueueDamage(int32 InstanceIndex, uint8 LODLevel, float Damage, bool bFromPlayer = false,
		const FVector& HitLocation = FVector::ZeroVector, const FVector& HitNormal = FVector::ForwardVector)
	{
		DamageQueue.Add({ InstanceIndex, LODLevel, Damage, static_cast<uint8>(bFromPlayer ? 1u : 0u), HitLocation, HitNormal });
	}

	/** 이 프레임의 플레이어 킬 수 — DamageSystem::Tick 이후 갱신 */
	int32 FramePlayerKillCount = 0;

	int32 FindLODIndexByHISM(const class UInstancedStaticMeshComponent* InHISM) const;

protected:
	entt::registry Registry;
	TArray<entt::entity> InstanceToEntityPerLOD[HISM_LOD_COUNT];
	TArray<FDamageEvent> DamageQueue;
	class UInstancedStaticMeshComponent* HISMRefs[HISM_LOD_COUNT] = {};

	float AttackRange = 150.f;

	/** Rush 웨이포인트 위치 배열 — 링크 순서대로 캐싱 */
	TArray<FVector> CachedWaypoints;

	/** 웨이포인트 도착 판정 반경 */
	float WaypointAcceptRadius = 500.f;

	TWeakObjectPtr<class UWorld> CachedWorld;
	uint8 bIsActive : 1 = false;

public:
	uint8 bWaypointsCollected : 1 = false;

protected:
	uint32 FrameCounter = 0;
};
