#include "UI/Widget/ItemBox/TPSItemBoxWidget.h"
#include "Component/Action/TPSPlayerInteractionComponent.h"
#include "Components/Button.h"

DEFINE_LOG_CATEGORY(LogItemBoxWidget);

void UTPSItemBoxWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ensure(CloseButton))
	{
		if (!CloseButton->OnClicked.IsBound())
		{
			CloseButton->OnClicked.AddDynamic(this, &UTPSItemBoxWidget::OnCloseButtonClicked);
		}
	}
}

void UTPSItemBoxWidget::NativeDestruct()
{
	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UTPSItemBoxWidget::OnCloseButtonClicked);
	}

	Super::NativeDestruct();
}

void UTPSItemBoxWidget::OnCloseButtonClicked()
{
	UE_LOG(LogItemBoxWidget, Log, TEXT("[OnCloseButtonClicked] Close button pressed"));

	UTPSPlayerInteractionComponent* pInteractionComponent = InteractionComponentRef.Get();
	if (ensure(pInteractionComponent))
	{
		pInteractionComponent->CloseInteraction();
	}
}