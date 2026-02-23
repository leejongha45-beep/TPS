#include "Actor/ItemBox/TPSItemBoxInteractionComponent.h"
#include "Actor/ItemBox/TPSItemBox.h"
#include "Blueprint/UserWidget.h"
#include "Character/Component/Action/TPSPlayerInteractionComponent.h"
#include "Character/Player/TPSPlayer.h"
#include "UI/Widget/ItemBox/TPSItemBoxWidget.h"

void UTPSItemBoxInteractionComponent::ToggleItemBox(bool bInput, APlayerController* InputController)
{
	bIsOpening = bInput;

	if (bIsOpening)
	{
		// ① 위젯 생성 (최초 1회)
		ATPSItemBox* pItemBox = Cast<ATPSItemBox>(GetOwner());
		if (!ensure(pItemBox)) return;

		if (!ItemBoxWidgetInst)
		{
			if (!ensure(ItemBoxWidgetClass.Get())) return;

			APlayerController* pController = InputController;
			if (!ensure(pController)) return;

			ItemBoxWidgetInst = CreateWidget<UTPSItemBoxWidget>(pController, ItemBoxWidgetClass);
		}

		// ② 오너 참조 설정 + 뷰포트 추가
		if (ensure(ItemBoxWidgetInst.Get()))
		{
			ItemBoxWidgetInst->SetOwningItemBox(pItemBox);

			ATPSPlayer* pPlayer = Cast<ATPSPlayer>(pItemBox->GetInteractableInterface().GetObject());
			if (ensure(pPlayer))
			{
				ItemBoxWidgetInst->SetOwningPlayer(pPlayer);
			}

			ItemBoxWidgetInst->AddToViewport();
		}
	}
	else
	{
		// ③ 닫기 — 뷰포트에서 제거
		if (ItemBoxWidgetInst)
		{
			ItemBoxWidgetInst->RemoveFromParent();
		}
	}
}