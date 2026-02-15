#include "Actor/ItemBox/TPSItemBoxInteractionComponent.h"
#include "Actor/ItemBox/TPSItemBox.h"
#include "Blueprint/UserWidget.h"
#include "Component/Action/TPSPlayerInteractionComponent.h"
#include "Pawn/Character/Player/TPSPlayer.h"
#include "UI/Widget/ItemBox/TPSItemBoxWidget.h"

void UTPSItemBoxInteractionComponent::ToggleItemBox(bool bInput, APlayerController* InputController)
{
	bIsOpening = bInput;

	if (bIsOpening)
	{
		ATPSItemBox* pItemBox = Cast<ATPSItemBox>(GetOwner());
		if (!ensure(pItemBox)) return;

		if (!ItemBoxWidgetInst)
		{
			if (!ensure(ItemBoxWidgetClass)) return;

			APlayerController* pController = InputController;
			if (!ensure(pController)) return;

			ItemBoxWidgetInst = CreateWidget<UTPSItemBoxWidget>(pController, ItemBoxWidgetClass);
		}

		if (ensure(ItemBoxWidgetInst))
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
		if (ItemBoxWidgetInst)
		{
			ItemBoxWidgetInst->RemoveFromParent();
		}
	}
}