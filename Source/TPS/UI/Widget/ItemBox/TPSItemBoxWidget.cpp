#include "UI/Widget/ItemBox/TPSItemBoxWidget.h"
#include "Component/Action/TPSPlayerInteractionComponent.h"
#include "Pawn/Character/Player/TPSPlayer.h"
#include "Components/Button.h"

DEFINE_LOG_CATEGORY(LogItemBoxWidget);

void UTPSItemBoxWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ensure(CloseButton))
	{
		CloseButton->OnClicked.AddDynamic(this, &UTPSItemBoxWidget::OnCloseButtonClicked);
	}
}

void UTPSItemBoxWidget::OnCloseButtonClicked()
{
	UE_LOG(LogItemBoxWidget, Log, TEXT("[OnCloseButtonClicked] Close button pressed"));

	APlayerController* pController = GetOwningPlayer();
	if (!ensure(pController)) return;

	ATPSPlayer* pPlayer = Cast<ATPSPlayer>(pController->GetPawn());
	if (!ensure(pPlayer)) return;

	UTPSPlayerInteractionComponent* pInteractionComponent = pPlayer->GetInteractionComponent();
	if (ensure(pInteractionComponent))
	{
		pInteractionComponent->CloseInteraction();
	}
}
