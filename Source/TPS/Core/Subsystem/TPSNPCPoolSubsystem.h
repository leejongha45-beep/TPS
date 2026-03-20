#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TPSNPCPoolSubsystem.generated.h"

/**
 * NPC 솔져 오브젝트 풀 서브시스템
 * - 프로젝타일 풀과 동일 패턴: Pre-Spawn + Get/Return
 * - 초기 50명 스폰, 최대 100명 풀링
 * - 비활성 NPC: Hidden + Tick 비활성 + 맵 외부 배치
 */
UCLASS()
class TPS_API UTPSNPCPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	/** 풀에서 비활성 NPC 1명 가져오기 — 위치 설정 후 Activate 호출 필요 */
	class ATPSSoldierNPC* GetNPC();

	/** 사용 완료된 NPC를 풀에 반환 */
	void ReturnNPC(class ATPSSoldierNPC* InNPC);

	/** NPC 활성화 — 위치 설정 + 가시성/콜리전/틱 ON */
	void ActivateNPC(class ATPSSoldierNPC* InNPC, const FVector& InLocation);

	/** NPC 비활성화 — 풀 대기 상태로 전환 */
	void DeactivateNPC(class ATPSSoldierNPC* InNPC);

	/** 현재 풀 여유분 */
	int32 GetAvailableCount() const { return Pool.Num(); }

	/** NPC BP 클래스 설정 (에디터에서 호출 or 외부 설정) */
	void SetNPCClass(TSubclassOf<class ATPSSoldierNPC> InClass) { NPCClass = InClass; }

protected:
	/** NPC InCount명 즉시 스폰 (비활성 상태) */
	void SpawnBatch(int32 InCount);

	/** 타이머 콜백 — 배치 단위로 나머지 스폰 */
	void DeferredSpawn();

private:
	/** 비활성 NPC 풀 */
	UPROPERTY()
	TArray<TObjectPtr<class ATPSSoldierNPC>> Pool;

	/** NPC BP 클래스 — DefaultGame.ini 또는 SwarmSubsystem에서 주입 */
	UPROPERTY()
	TSubclassOf<class ATPSSoldierNPC> NPCClass;

	/** NPC BP 클래스 경로 — 소프트 로딩용 */
	FSoftClassPath NPCClassPath = FSoftClassPath(TEXT("/Game/BP_Class/Pawn/Character/NPC/BP_TPSSoldierNPC.BP_TPSSoldierNPC_C"));

	FTimerHandle DeferredSpawnTimerHandle;

	int32 InitialSpawnCount = 50;
	int32 MaxPoolSize = 100;
	int32 DeferredSpawnBatchSize = 5;
	int32 TotalSpawnedCount = 0;
};
