// TPSEnemyTypeDataAsset.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TPSEnemyTypeDataAsset.generated.h"

/**
 * 단일 적 타입의 기본 스탯과 스폰 조건을 정의하는 DataAsset.
 * 에디터 Content Browser에서 생성하여 WaveSettings에 등록한다.
 */
UCLASS(BlueprintType)
class TPS_API UTPSEnemyTypeDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/* ── 식별 ── */
	UPROPERTY(EditDefaultsOnly, Category = "Identity")
	FName TypeName = NAME_None;

	/* ── 전투 스탯 ── */
	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (ClampMin = "1.0"))
	float MaxHealth = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float MaxSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float AttackDamage = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (ClampMin = "0.1"))
	float AttackCooldown = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float AttackRange = 150.f;

	/* ── 스폰 조건 ── */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn", meta = (ClampMin = "0"))
	int32 MinWaveLevel = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn", meta = (ClampMin = "1"))
	int32 SpawnWeight = 10;

	/* ── 렌더링 ── */

	/** LOD별 메시 배열 — [0] Near(풀메시), [1] Mid(심플), [2] Far(큐브) */
	UPROPERTY(EditDefaultsOnly, Category = "Rendering")
	TArray<TSoftObjectPtr<class UStaticMesh>> LODMeshes;

	/** 메시 정면 보정용 Yaw 오프셋 (도) — 메시 기본 정면이 +X가 아닐 때 사용 */
	UPROPERTY(EditDefaultsOnly, Category = "Rendering")
	float MeshYawOffset = 0.f;
};
