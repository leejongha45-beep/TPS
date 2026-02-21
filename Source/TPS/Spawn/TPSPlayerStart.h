#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "TPSPlayerStart.generated.h"

/**
 * 커스텀 PlayerStart — 미니맵 스폰 선택 UI용
 * - 레벨에 다수 배치, 플레이어가 UI에서 선택
 * - 기지 파괴 시 Deactivate()로 선택 불가 처리
 * - Activate/Deactivate 패턴으로 선택 가능 여부 제어
 */
UCLASS()
class TPS_API ATPSPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	ATPSPlayerStart(const FObjectInitializer& ObjectInitializer);

	FORCEINLINE bool IsActive() const { return bIsActive; }
	FORCEINLINE FText GetDisplayName() const { return DisplayName; }

	/** 비활성화 — UI에서 선택 불가 */
	void Deactivate();

	/** 활성화 — UI에서 선택 가능 */
	void Activate();

protected:
	/** UI에 표시할 이름 (예: "전초기지 A", "본부") */
	UPROPERTY(EditInstanceOnly, Category = "SpawnPoint")
	FText DisplayName;

	/** 활성화 상태 — false면 UI에서 선택 불가 */
	uint8 bIsActive : 1 = true;

#if WITH_EDITORONLY_DATA
	/** 에디터 빌보드 아이콘 */
	UPROPERTY()
	TObjectPtr<class UBillboardComponent> BillboardComponentInst;
#endif
};
