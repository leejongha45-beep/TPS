#pragma once

#include "CoreMinimal.h"
#include "SurfaceType.generated.h"

/**
 * 바닥 재질 타입
 * - UE PhysicalSurface (SurfaceType1~8)와 1:1 매핑
 * - FootstepHelper에서 DataTable RowName으로 사용
 * - Project Settings → Physics → Surface Types 순서와 일치시킬 것
 */
UENUM(BlueprintType)
enum class ESurfaceType : uint8
{
	Default		UMETA(DisplayName = "Default"),  /** 매핑 실패 시 fallback */
	Concrete	UMETA(DisplayName = "Concrete"), /** 콘크리트/아스팔트 */
	Dirt		UMETA(DisplayName = "Dirt"),      /** 흙/모래 */
	Grass		UMETA(DisplayName = "Grass"),     /** 잔디/풀밭 */
	Snow		UMETA(DisplayName = "Snow"),      /** 눈 */
	Metal		UMETA(DisplayName = "Metal"),     /** 금속/철판 */
	Water		UMETA(DisplayName = "Water"),     /** 물/웅덩이 */
	Wood		UMETA(DisplayName = "Wood"),      /** 나무/목재 */
	Mud			UMETA(DisplayName = "Mud"),       /** 진흙 */
	MAX			UMETA(Hidden)
};
