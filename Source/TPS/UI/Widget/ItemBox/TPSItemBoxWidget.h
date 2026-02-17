#pragma once

#include "CoreMinimal.h"
#include "Utils/UENUM/WeaponType.h"
#include "Blueprint/UserWidget.h"
#include "TPSItemBoxWidget.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogItemBoxWidget, Log, All);

/**
 * 아이템 박스 위젯
 * - 무기 선택 UI (라이플 버튼 등)
 * - 닫기 버튼으로 상호작용 종료
 * - NativeConstruct에서 버튼 콜백 바인딩
 */
UCLASS()
class TPS_API UTPSItemBoxWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FORCEINLINE void SetOwningItemBox(class ATPSItemBox* InItemBox) { OwningItemBoxRef = InItemBox; }
	FORCEINLINE void SetOwningPlayer(class ATPSPlayer* InPlayer) { OwningPlayerRef = InPlayer; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 닫기 버튼 클릭 콜백 */
	UFUNCTION()
	void OnCloseButtonClicked();

	/** 닫기 버튼 (BP 바인딩) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> CloseButton;

	/** 라이플 선택 버튼 클릭 콜백 */
	UFUNCTION()
	void OnRifleButtonClicked();

	/** 라이플 버튼 (BP 바인딩) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> RifleButton;

	/** 소유 아이템 박스 참조 (WeakPtr) */
	TWeakObjectPtr<class ATPSItemBox> OwningItemBoxRef;

	/** 소유 플레이어 참조 (WeakPtr) */
	TWeakObjectPtr<class ATPSPlayer> OwningPlayerRef;
};