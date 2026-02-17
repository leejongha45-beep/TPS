#include "UI/Widget/ItemBox/TPSItemBoxWidget.h"
#include "Actor/ItemBox/TPSItemBox.h"
#include "Component/Action/TPSPlayerInteractionComponent.h"
#include "Components/Button.h"
#include "Pawn/Character/Player/TPSPlayer.h"

DEFINE_LOG_CATEGORY(LogItemBoxWidget);

void UTPSItemBoxWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// ① 닫기 버튼 바인딩
	if (ensure(CloseButton))
	{
		if (!CloseButton->OnClicked.IsBound())
		{
			CloseButton->OnClicked.AddDynamic(this, &UTPSItemBoxWidget::OnCloseButtonClicked);
		}
	}

	// ② 라이플 버튼 바인딩
	if (ensure(RifleButton))
	{
		if (!RifleButton->OnClicked.IsBound())
		{
			RifleButton->OnClicked.AddDynamic(this, &UTPSItemBoxWidget::OnRifleButtonClicked);
		}
	}
}

void UTPSItemBoxWidget::NativeDestruct()
{
	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UTPSItemBoxWidget::OnCloseButtonClicked);
	}

	if (RifleButton)
	{
		RifleButton->OnClicked.RemoveDynamic(this, &UTPSItemBoxWidget::OnRifleButtonClicked);
	}

	Super::NativeDestruct();
}

void UTPSItemBoxWidget::OnCloseButtonClicked()
{
	UE_LOG(LogItemBoxWidget, Log, TEXT("[OnCloseButtonClicked] Close button pressed"));

	ATPSPlayer* pPlayer = OwningPlayerRef.Get();
	if (!ensure(pPlayer)) return;

	UTPSPlayerInteractionComponent* pPlayerInteractionComponent = pPlayer->GetInteractionComponent();
	if (ensure(pPlayerInteractionComponent))
	{
		pPlayerInteractionComponent->CloseInteraction();
	}
}

void UTPSItemBoxWidget::OnRifleButtonClicked()
{
	UE_LOG(LogItemBoxWidget, Log, TEXT("[OnRifleButtonClicked] Rifle button pressed"));

	ATPSItemBox* pOwnerItemBox = OwningItemBoxRef.Get();
	if (ensure(pOwnerItemBox))
	{
		pOwnerItemBox->SpawnWeapon(EWeaponType::Rifle);
	}
}