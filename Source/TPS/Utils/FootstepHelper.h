#pragma once

#include "CoreMinimal.h"
#include "Utils/UENUM/SurfaceType.h"

struct FFootstepSoundRow;

/**
 * 발소리 시스템 유틸리티
 * - LineTrace로 바닥 재질 감지
 * - DataTable에서 재질별 사운드 조회
 * - 사운드 재생
 *
 * 리팩토링 시 이 클래스 내부만 교체하면
 * AnimNotify는 수정 불필요
 */
class TPS_API FFootstepHelper
{
public:
	/**
	 * 발소리 재생 진입점
	 * AnimNotify에서 이 함수만 호출
	 *
	 * @param MeshComp			SkeletalMeshComponent
	 * @param FootBoneName		발 본 이름 (foot_l / foot_r)
	 * @param DataTableAsset	발소리 DataTable
	 * @param TraceDistance		LineTrace 거리
	 */
	static void PlayFootstepSound(
		USkeletalMeshComponent* MeshComp,
		const FName& FootBoneName,
		class UDataTable* DataTableAsset,
		float TraceDistance
	);

private:
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

	/** DataTable에서 해당 SurfaceType의 Row 조회 (없으면 Default fallback) */
	static const FFootstepSoundRow* FindFootstepRow(
		const UDataTable* DataTableAsset,
		ESurfaceType SurfaceType
	);
};
