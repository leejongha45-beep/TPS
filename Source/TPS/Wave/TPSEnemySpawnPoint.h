#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "TPSEnemySpawnPoint.generated.h"

/**
 * 적 스폰 지점 액터
 * - 레벨에 배치하여 적 스폰 위치를 지정
 * - SpawnRadius 내 랜덤 분산 스폰
 * - GameplayTag로 웨이브별 필터링
 * - bIsEnemyHQ로 본진 포인트 구분 (최종 단계에서만 사용)
 * - 기지 파괴 시 Deactivate()로 비활성화 → 추가 스폰 차단
 */
UCLASS()
class TPS_API ATPSEnemySpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	ATPSEnemySpawnPoint();

	/** 포인트 위치 + SpawnRadius 내 랜덤 오프셋 반환 (XY만, Z는 SnapToGround에서 보정) */
	FVector GetRandomLocationInRadius() const;

	FORCEINLINE float GetSpawnRadius() const { return SpawnRadius; }
	FORCEINLINE FGameplayTag GetSpawnPointTag() const { return SpawnPointTag; }
	FORCEINLINE bool IsActive() const { return bIsActive; }
	FORCEINLINE bool IsEnemyHQ() const { return bIsEnemyHQ; }

	/** 비활성화 — 서브시스템 쿼리에서 제외 (추가 스폰 차단) */
	void Deactivate();

	/** 활성화 — 서브시스템 쿼리에 다시 포함 */
	void Activate();

protected:
	/** 포인트 주변 랜덤 분산 반경 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "SpawnPoint", meta = (ClampMin = "0"))
	float SpawnRadius = 500.f;

	/** 웨이브별 필터링 태그 (예: "SpawnPoint.Outpost", "SpawnPoint.HQ") */
	UPROPERTY(EditDefaultsOnly, Category = "SpawnPoint")
	FGameplayTag SpawnPointTag;

	/** 적 본진 포인트 여부 — 최종 단계에서만 쿼리에 포함 */
	UPROPERTY(EditInstanceOnly, Category = "SpawnPoint")
	uint8 bIsEnemyHQ : 1 = false;

	/** 활성화 상태 — false면 서브시스템 쿼리에서 제외 */
	uint8 bIsActive : 1 = true;

#if WITH_EDITORONLY_DATA
	/** 에디터 빌보드 아이콘 */
	UPROPERTY()
	TObjectPtr<class UBillboardComponent> BillboardComponentInst;

	/** SpawnRadius 시각화용 구체 */
	UPROPERTY()
	TObjectPtr<class USphereComponent> DebugRadiusSphereInst;
#endif
};
