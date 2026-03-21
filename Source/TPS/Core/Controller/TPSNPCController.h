#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Utils/Interface/Action/AIControllable.h"
#include "Utils/TickFunctions/FAIControlTickFunction.h"
#include "TPSNPCController.generated.h"

UCLASS()
class TPS_API ATPSNPCController : public AAIController, public IAIControllable
{
	GENERATED_BODY()

public:
	ATPSNPCController();

	/** 풀에서 꺼낼 때 상태 리셋 — 웨이포인트 인덱스/이동 플래그 초기화 */
	void ResetAIState();

	/** AI 틱 활성화/비활성화 — 풀 Activate/Deactivate용 */
	void SetAITickEnabled(bool bEnabled);

protected:
	virtual void RegisterActorTickFunctions(bool bRegister) override;
	virtual void AI_Control_Tick(float DeltaTime) override;

	FAIControlTickFunction AIControlTick;

	virtual void BeginPlay() override;

	/** 범위 내 가장 가까운 적 탐색 → 타겟 위치 반환, 없으면 false */
	bool FindNearestEnemy(FVector& OutLocation) const;

	/** NPC 웨이포인트 링크 순회 → CachedWaypoints 적재 */
	void CollectWaypoints();

	/** 타겟 방향으로 회전 */
	void RotateToTarget(const FVector& TargetLocation, float DeltaTime);

	/** IFireable 인터페이스로 사격 시작/중지/재장전 */
	void CommandFire();
	void CommandCeaseFire();
	void CommandReload();

	UPROPERTY(EditAnywhere, Category = "AI")
	float DetectionRadius = 10000.f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float FireRange = 3000.f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float AimInterpSpeed = 5.f;

	/** 웨이포인트 위치 배열 */
	TArray<FVector> CachedWaypoints;
	float WaypointAcceptRadius = 5000.f;
	int32 CurrentWaypointIndex = 0;

	bool bIsMovingToWaypoint = false;

	/** 감지 결과 캐싱 — N프레임마다 갱신 */
	FVector CachedEnemyLocation = FVector::ZeroVector;
	bool bHasCachedEnemy = false;
	int32 AIFrameCounter = 0;

	/** 감지 갱신 주기 (프레임 수) */
	static constexpr int32 DetectionInterval = 5;
};
