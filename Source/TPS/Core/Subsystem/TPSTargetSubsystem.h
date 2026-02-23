#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Utils/Interface/Data/Targetable.h"
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
	/** ITargetable 등록 — BeginPlay에서 호출 */
	void RegisterTargetableActor(const TScriptInterface<ITargetable>& TargetableActor);

	/** ITargetable 해제 — EndPlay/OnDestroyed에서 호출 */
	void UnregisterTargetableActor(const TScriptInterface<ITargetable>& TargetableActor);

	/** 등록된 ITargetable 목록 반환 (GameThread 전용) */
	FORCEINLINE const TArray<TScriptInterface<ITargetable>>& GetTargetableActors() const
	{
		checkSlow(IsInGameThread());
		return TargetableActors;
	}

	/** 아군 기지 위치 설정 */
	void SetAllyBaseLocation(const FVector& InLocation);

	/** 아군 기지 위치 반환 */
	FORCEINLINE const FVector& GetAllyBaseLocation() const { return AllyBaseLocation; }

private:
	/** 등록된 ITargetable 목록 */
	TArray<TScriptInterface<ITargetable>> TargetableActors;

	/** 아군 기지 위치 (기본 진격 목표) */
	FVector AllyBaseLocation = FVector::ZeroVector;
};