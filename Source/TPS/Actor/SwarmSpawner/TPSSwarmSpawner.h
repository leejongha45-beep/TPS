#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/Subsystem/TPSSwarmSubsystem.h"
#include "TPSSwarmSpawner.generated.h"

/**
 * 군집 스포너 — 레벨에 배치하여 군집 생성 책임
 * - SwarmSubsystem이 명령 → 스포너가 FSwarm 생성/반환
 * - 팀, 병력 수, 유닛 스탯을 에디터에서 설정
 */
UCLASS()
class TPS_API ATPSSwarmSpawner : public AActor
{
	GENERATED_BODY()

public:
	ATPSSwarmSpawner();

	void StartSpawning();
	void StopSpawning();

	/** 스포너 설정대로 군집 1개 생성 */
	FSwarm SpawnSwarm() const;

	/** 지정 병력수로 군집 생성 (빅웨이브용) */
	FSwarm SpawnSwarm(int32 OverrideTroopCount) const;

	FORCEINLINE bool IsActive() const { return bIsActive; }
	FORCEINLINE ESwarmTeam GetTeam() const { return Team; }
	FORCEINLINE float GetSpawnInterval() const { return SpawnInterval; }

	/** 마지막 스폰 시각 (SwarmSubsystem이 관리) */
	float LastSpawnTime = 0.f;

protected:
	/** 소속 팀 */
	UPROPERTY(EditAnywhere, Category = "Swarm")
	ESwarmTeam Team = ESwarmTeam::Enemy;

	/** 군집 스폰 주기 (초) */
	UPROPERTY(EditAnywhere, Category = "Swarm", meta = (ClampMin = "1.0"))
	float SpawnInterval = 5.f;

	/** 군집당 병력 수 */
	UPROPERTY(EditAnywhere, Category = "Swarm", meta = (ClampMin = "1"))
	int32 TroopCount = 50;

	/** 유닛 최대 HP */
	UPROPERTY(EditAnywhere, Category = "Swarm|Stats")
	float UnitMaxHP = 50.f;

	/** 유닛 공격력 */
	UPROPERTY(EditAnywhere, Category = "Swarm|Stats")
	float AttackPower = 1.f;

	/** 유닛 이동속도 */
	UPROPERTY(EditAnywhere, Category = "Swarm|Stats")
	float MoveSpeed = 300.f;

	/** 웨이포인트 경로 인덱스 — 어떤 루트로 군집을 보낼지 */
	UPROPERTY(EditAnywhere, Category = "Swarm")
	int32 RouteIndex = 0;

	/** 스폰 반경 (스포너 위치 주변 랜덤 오프셋) */
	UPROPERTY(EditAnywhere, Category = "Swarm")
	float SpawnRadius = 500.f;

	/** 루트 씬 컴포넌트 */
	UPROPERTY(VisibleAnywhere, Category = "Swarm")
	TObjectPtr<class USceneComponent> SceneRoot;

#if WITH_EDITORONLY_DATA
	/** 에디터 빌보드 */
	UPROPERTY(VisibleAnywhere, Category = "Swarm")
	TObjectPtr<class UBillboardComponent> SpriteComponent;
#endif

private:
	/** 스폰 위치 계산 (액터 위치 + 반경 내 랜덤 오프셋) */
	FVector CalcSpawnPosition() const;

	uint8 bIsActive : 1 = false;
};
