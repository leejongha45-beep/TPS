#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPSItemBoxInteractionComponent.generated.h"

/**
 * 아이템 박스 상호작용 컴포넌트
 * - 아이템 박스 UI 위젯 열기/닫기 관리
 * - ToggleItemBox로 토글 (bInput: true=열기, false=닫기)
 */
UCLASS()
class TPS_API UTPSItemBoxInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** 아이템 박스 UI 토글 */
	void ToggleItemBox(bool bInput, APlayerController* InputController);

	FORCEINLINE bool GetIsOpen() const { return bIsOpening; }

protected:
	/** 아이템 박스 위젯 BP 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UTPSItemBoxWidget> ItemBoxWidgetClass;

	/** 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<class UTPSItemBoxWidget> ItemBoxWidgetInst;

	uint8 bIsOpening : 1 = false;
};