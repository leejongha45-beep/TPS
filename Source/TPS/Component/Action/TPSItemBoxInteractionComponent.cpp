#include "Component/Action/TPSItemBoxInteractionComponent.h"
#include "UI/Widget/ItemBox/TPSItemBoxWidget.h"
#include "Actor/ItemBox/TPSItemBox.h"
#include "Blueprint/UserWidget.h"

void UTPSItemBoxInteractionComponent::ToggleItemBox()
{
	if (bIsOpen)
	{
		if (ItemBoxWidgetInst)
		{
			ItemBoxWidgetInst->RemoveFromParent();
		}

		bIsOpen = false;
	}
	else
	{
		ATPSItemBox* pItemBox = Cast<ATPSItemBox>(GetOwner());
		if (!ensure(pItemBox)) return;

		if (!ItemBoxWidgetInst)
		{
			if (!ensure(ItemBoxWidgetClass)) return;

			APlayerController* pController = GetWorld()->GetFirstPlayerController();
			if (!ensure(pController)) return;

			ItemBoxWidgetInst = CreateWidget<UTPSItemBoxWidget>(pController, ItemBoxWidgetClass);
		}

		if (ensure(ItemBoxWidgetInst))
		{
			ItemBoxWidgetInst->SetOwningItemBox(pItemBox);
			ItemBoxWidgetInst->AddToViewport();
			bIsOpen = true;
		}
	}
}
