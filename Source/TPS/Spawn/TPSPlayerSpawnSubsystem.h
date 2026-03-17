#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TPSPlayerSpawnSubsystem.generated.h"

/** 플레이어 스폰 포인트 비활성화 통보 (비활성화된 포인트) */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerSpawnDeactivated, class ATPSPlayerStart*);

/**
 * 플레이어 스폰 포인트 관리 월드 서브시스템
 * - BeginPlay 시 레벨의 모든 ATPSPlayerStart 캐싱
 * - UI가 활성 포인트를 조회하여 미니맵에 마커 표시
 * - 기지 파괴 시 비활성화 API 제공
 */
UCLASS()
class TPS_API UTPSPlayerSpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ UWorldSubsystem
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;
	//~ End UWorldSubsystem

	/** 활성 상태인 플레이어 스폰 포인트 배열 반환 */
	TArray<class ATPSPlayerStart*> GetActiveSpawnPoints() const;

	/** 특정 포인트 비활성화 (기지 파괴 시 호출) */
	void DeactivateSpawnPoint(class ATPSPlayerStart* InPoint);

	/** 비활성화 통보 델리게이트 */
	FOnPlayerSpawnDeactivated OnPlayerSpawnDeactivatedDelegate;

protected:
	/** 레벨의 모든 ATPSPlayerStart 캐시 */
	UPROPERTY()
	TArray<class ATPSPlayerStart*> CachedSpawnPoints;

private:
	/** 레벨에서 ATPSPlayerStart 수집 */
	void CollectSpawnPoints(UWorld* World);
};