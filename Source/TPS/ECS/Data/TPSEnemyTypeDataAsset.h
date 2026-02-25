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
	UPROPERTY(EditDefaultsOnly, Category = "Rendering")
	uint8 MeshTypeIndex = 0;
};
