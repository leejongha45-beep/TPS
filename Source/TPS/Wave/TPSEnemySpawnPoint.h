// TPSEnemySpawnPoint.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPSEnemySpawnPoint.generated.h"

/**
 * 레벨에 배치하는 적 스폰 포인트.
 * TPSWaveSubsystem이 월드에서 수집하여 랜덤 위치로 사용한다.
 */
UCLASS(Blueprintable)
class TPS_API ATPSEnemySpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	ATPSEnemySpawnPoint();

	/** 이 스폰포인트의 월드 위치 반환 */
	FORCEINLINE FVector GetSpawnLocation() const { return GetActorLocation(); }

	/** 스폰 시 랜덤 반경 — 0이면 정확히 액터 위치, >0이면 반경 내 랜덤 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0.0"))
	float SpawnRadius = 1000.f;

	/** 비활성화 시 스폰 대상에서 제외 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	uint8 bEnabled : 1 = true;

protected:
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "Spawn")
	TObjectPtr<class UBillboardComponent> SpriteComponent = nullptr;
#endif
};
