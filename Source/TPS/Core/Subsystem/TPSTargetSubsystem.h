#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TPSTargetSubsystem.generated.h"

/**
 * ITargetable 등록/해제 중계 서브시스템
 * - 월드 생명주기에 종속 — 맵 전환 시 자동 정리
 * - BeginPlay에서 RegisterTargetableActor, OnDestroyed에서 UnregisterTargetableActor
 * - PlayerLocationProcessor가 100ms 주기로 읽어서 SharedFragment에 복사
 */
UCLASS()
class TPS_API UTPSTargetSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** ITargetable 액터 등록 — BeginPlay에서 호출 */
	void RegisterTargetableActor(AActor* TargetableActor);

	/** ITargetable 액터 해제 — OnDestroyed에서 호출 */
	void UnregisterTargetableActor(AActor* TargetableActor);

	/** 등록된 ITargetable 액터 목록 반환 (GameThread 전용) */
	FORCEINLINE const TArray<TWeakObjectPtr<AActor>>& GetTargetableActors() const
	{
		return TargetableActors;
	}

	/** 아군 기지 위치 설정 */
	void SetAllyBaseLocation(const FVector& InLocation);

	/** 아군 기지 위치 반환 */
	FORCEINLINE const FVector& GetAllyBaseLocation() const { return AllyBaseLocation; }

private:
	/** 등록된 ITargetable 액터 목록 */
	TArray<TWeakObjectPtr<AActor>> TargetableActors;

	/** 아군 기지 위치 (기본 진격 목표) */
	FVector AllyBaseLocation = FVector::ZeroVector;
};
