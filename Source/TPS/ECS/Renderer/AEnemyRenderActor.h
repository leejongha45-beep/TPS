#pragma once

#include "GameFramework/Actor.h"
#include "AEnemyRenderActor.generated.h"

/** LOD 레벨 수 — Near(0), Mid(1), Far(2) */
inline constexpr int32 HISM_LOD_COUNT = 3;

/**
 * 적 렌더 액터 — LOD별 ISM 기반 대량 인스턴스 렌더링
 * - ISM은 InitializeISMs에서 NewObject로 동적 생성 (Mass Entity 패턴)
 * - BP 클래스로 사용, WaveSettings에서 RenderActorClass로 참조
 * - EnemyManagerSubsystem이 소유, Scheduler에 ISM 배열 참조 전달
 */
UCLASS()
class AEnemyRenderActor : public AActor
{
	GENERATED_BODY()

public:
	AEnemyRenderActor();

	/**
	 * LOD별 ISM 동적 생성 — SpawnActor 직후 호출
	 * @param Meshes  LOD별 StaticMesh 배열 (nullptr 허용)
	 * @param Count   배열 크기
	 */
	void InitializeISMs(class UStaticMesh* Meshes[], int32 Count);

	/** LOD 인덱스로 ISM 접근 (0=Near, 1=Mid, 2=Far) */
	class UInstancedStaticMeshComponent* GetISMComponent(int32 LODIndex) const;

protected:
	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<class UInstancedStaticMeshComponent>> ISMComponents;
};
