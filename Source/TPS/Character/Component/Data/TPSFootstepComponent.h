#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/StaticArray.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Utils/UENUM/SurfaceType.h"
#include "TPSFootstepComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(FootstepLog, Log, All);

/**
 * 발소리 데이터 컴포넌트
 * - BeginPlay에서 DataTable 전체를 TStaticArray로 캐싱 (O(1) 인덱스 조회)
 * - AnimNotify에서 PlayFootstepSound()만 호출하면 됨
 * - LineTrace → 바닥 재질 감지 → 캐시 조회 → Walk/Run 사운드 재생
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TPS_API UTPSFootstepComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTPSFootstepComponent();

	/**
	 * 발소리 재생 진입점
	 * AnimNotify에서 이 함수만 호출
	 *
	 * @param MeshComp		SkeletalMeshComponent
	 * @param FootBoneName	발 본 이름 (foot_l / foot_r)
	 * @param TraceDistance	LineTrace 거리 (cm)
	 */
	void PlayFootstepSound(
		class USkeletalMeshComponent* MeshComp,
		const FName& FootBoneName,
		float TraceDistance
	);

protected:
	virtual void BeginPlay() override;

	/** 발소리 DataTable (RowType: FFootstepSoundRow) — BP에서 설정 */
	UPROPERTY(EditDefaultsOnly, Category = "Footstep")
	TObjectPtr<class UDataTable> FootstepDataTableAsset;

private:
	/** BeginPlay에서 DataTable 전체를 캐싱 */
	void CacheFootstepRows();

	/** 캐시에서 해당 SurfaceType의 Row 조회 (O(1)), 미스 시 Default fallback */
	const struct FFootstepSoundRow* FindCachedRow(ESurfaceType SurfaceType) const;

	/** LineTrace로 바닥 PhysicalMaterial의 SurfaceType 감지 */
	static ESurfaceType DetectSurfaceType(
		UWorld* World,
		const FVector& TraceStart,
		float TraceDistance,
		AActor* IgnoreActor,
		FVector& OutImpactPoint
	);

	/** UE PhysicalSurface → ESurfaceType 매핑 */
	static ESurfaceType ConvertPhysicalSurface(EPhysicalSurface InSurface);

	/** ESurfaceType별 캐싱된 Row 포인터 (DataTable UPROPERTY가 GC 수명 보장) */
	TStaticArray<const struct FFootstepSoundRow*, static_cast<uint32>(ESurfaceType::MAX)> CachedRows;
};