#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TPSEnemyISMSubsystem.generated.h"

/**
 * 적 ISM(Instanced Static Mesh) 렌더링 관리 서브시스템
 * - LOD ISM 단계 적의 시각화 담당
 * - AddInstance / RemoveInstance / UpdateTransform
 * - 같은 메시를 한 번의 드로우 콜로 수백 마리 렌더링
 */
UCLASS()
class TPS_API UTPSEnemyISMSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** 서브시스템 초기화 — ISM 컴포넌트 생성 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 서브시스템 해제 — ISM 컴포넌트 정리 */
	virtual void Deinitialize() override;

	/** ISM 인스턴스 추가 → 인스턴스 인덱스 반환 */
	int32 AddInstance(const FTransform& InTransform);

	/** ISM 인스턴스 제거 (인덱스 기반 — 실제 삭제 대신 재활용) */
	void RemoveInstance(int32 InInstanceIndex);

	/** ISM 인스턴스 위치 갱신 */
	void UpdateInstanceTransform(int32 InInstanceIndex, const FTransform& InTransform);

private:
	/** ISM 컴포넌트를 보유하는 Actor */
	UPROPERTY()
	TObjectPtr<AActor> ISMOwnerActor;

	/** ISM 컴포넌트 */
	UPROPERTY()
	TObjectPtr<class UInstancedStaticMeshComponent> ISMComponent;

	/** 제거된 인스턴스 인덱스 재활용 풀 */
	TArray<int32> FreeIndices;
};
