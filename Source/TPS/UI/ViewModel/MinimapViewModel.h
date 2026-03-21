#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Utils/UENUM/SwarmTeam.h"
#include "MinimapViewModel.generated.h"

/** 미니맵에 표시할 마커 1개의 가공된 데이터 */
USTRUCT()
struct FMinimapMarkerData
{
	GENERATED_BODY()

	FVector2D Position = FVector2D::ZeroVector;
	FLinearColor Color = FLinearColor::White;
	float Size = 10.f;
	bool bVisible = true;
};

/**
 * 미니맵 뷰모델
 * - SwarmSubsystem이 소유, Tick에서 Update() 호출 (푸시)
 * - Widget(View)은 Getter로 읽기만 수행
 * - 월드 좌표 → 2D 변환까지 ViewModel이 담당
 */
UCLASS()
class TPS_API UMinimapViewModel : public UObject
{
	GENERATED_BODY()

public:
	/** SwarmSubsystem Tick에서 매 프레임 호출 */
	void Update(class UWorld* InWorld, const FVector2D& InMapCenter, float InMapExtent);

	/** 맵 범위 설정 */
	void SetMapBounds(const FVector2D& InCenter, float InExtent);

	// ──────────── Getter (Widget이 읽기만) ────────────
	FORCEINLINE const TArray<FMinimapMarkerData>& GetSwarmMarkers() const { return SwarmMarkers; }
	FORCEINLINE const TArray<FMinimapMarkerData>& GetBaseMarkers() const { return BaseMarkers; }
	FORCEINLINE const FVector2D& GetPlayerPosition() const { return PlayerPosition; }
	FORCEINLINE bool IsPlayerValid() const { return bPlayerValid; }

private:
	/** 월드 좌표 → 정규화 좌표 (0~1) 변환 */
	FVector2D WorldToNormalized(const FVector& WorldPos) const;

	TArray<FMinimapMarkerData> SwarmMarkers;
	TArray<FMinimapMarkerData> BaseMarkers;
	FVector2D PlayerPosition = FVector2D(0.5f, 0.5f);
	bool bPlayerValid = false;

	FVector2D MapWorldCenter = FVector2D::ZeroVector;
	float MapWorldExtent = 50000.f;

};
