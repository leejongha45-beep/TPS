// TPSWaveSettings.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "TPSWaveSettings.generated.h"

/**
 * 웨이브 시스템의 수식 기반 설정.
 * Project Settings → Game → TPS Wave 에서 조정 가능.
 *
 * 트리클/빅웨이브/스케일링/스폰위치/적타입 카테고리로 분리.
 * GetDefault<UTPSWaveSettings>() 로 어디서든 CDO 접근.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "TPS Wave"))
class TPS_API UTPSWaveSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/* ── 트리클 스폰 ── */

	/** 소그룹 스폰 주기 (초) */
	UPROPERTY(Config, EditAnywhere, Category = "Trickle", meta = (ClampMin = "0.5"))
	float TrickleSpawnInterval = 3.f;

	/** 소그룹당 적 수 */
	UPROPERTY(Config, EditAnywhere, Category = "Trickle", meta = (ClampMin = "1"))
	int32 TrickleGroupSize = 3;

	/** 그룹 내 적 간 스폰 딜레이 (초) */
	UPROPERTY(Config, EditAnywhere, Category = "Trickle", meta = (ClampMin = "0.0"))
	float TrickleGroupSpawnDelay = 0.5f;

	/* ── 빅웨이브 ── */

	/** 빅웨이브 발생 주기 (초) */
	UPROPERTY(Config, EditAnywhere, Category = "BigWave", meta = (ClampMin = "10.0"))
	float BigWavePeriod = 60.f;

	/** 빅웨이브 기본 적 수 */
	UPROPERTY(Config, EditAnywhere, Category = "BigWave", meta = (ClampMin = "1"))
	int32 BigWaveBaseCount = 30;

	/** 웨이브 레벨당 추가 적 수 */
	UPROPERTY(Config, EditAnywhere, Category = "BigWave", meta = (ClampMin = "0"))
	int32 BigWaveCountPerLevel = 10;

	/** 빅웨이브 그룹 분할 수 */
	UPROPERTY(Config, EditAnywhere, Category = "BigWave", meta = (ClampMin = "1"))
	int32 BigWaveGroupCount = 3;

	/** 빅웨이브 그룹 간 스폰 간격 (초) */
	UPROPERTY(Config, EditAnywhere, Category = "BigWave", meta = (ClampMin = "0.5"))
	float BigWaveGroupInterval = 5.f;

	/** 빅웨이브 경고 시간 (초) — Alert → Active 전환까지 */
	UPROPERTY(Config, EditAnywhere, Category = "BigWave", meta = (ClampMin = "0.0"))
	float BigWaveAlertDuration = 3.f;

	/* ── 스케일링 ── */

	/** 웨이브 레벨당 HP 배율 증가분 (1.0 + Level * 이 값) */
	UPROPERTY(Config, EditAnywhere, Category = "Scaling", meta = (ClampMin = "0.0"))
	float HealthMultiplierPerWave = 0.1f;

	/** 웨이브 레벨당 속도 배율 증가분 */
	UPROPERTY(Config, EditAnywhere, Category = "Scaling", meta = (ClampMin = "0.0"))
	float SpeedMultiplierPerWave = 0.02f;

	/** 웨이브 레벨당 공격력 배율 증가분 */
	UPROPERTY(Config, EditAnywhere, Category = "Scaling", meta = (ClampMin = "0.0"))
	float DamageMultiplierPerWave = 0.05f;

	/** 필드 동시 존재 최대 적 수 */
	UPROPERTY(Config, EditAnywhere, Category = "Scaling", meta = (ClampMin = "1"))
	int32 MaxEnemyCount = 3000;

	/* ── 렌더링 ── */

	/** 렌더 액터 BP 클래스 — ISM에 메시를 에디터에서 할당 */
	UPROPERTY(Config, EditAnywhere, Category = "Rendering")
	TSoftClassPtr<class AEnemyRenderActor> RenderActorClass;

	/* ── 적 타입 ── */

	/** 사용 가능한 적 타입 목록 (DataAsset) */
	UPROPERTY(Config, EditAnywhere, Category = "EnemyTypes")
	TArray<TSoftObjectPtr<class UTPSEnemyTypeDataAsset>> EnemyTypes;
};