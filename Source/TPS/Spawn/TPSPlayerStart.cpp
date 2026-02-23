#include "Spawn/TPSPlayerStart.h"
#include "Components/BillboardComponent.h"

ATPSPlayerStart::ATPSPlayerStart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	if (!BillboardComponentInst)
	{
		BillboardComponentInst = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
		if (ensure(BillboardComponentInst.Get()))
		{
			BillboardComponentInst->SetupAttachment(RootComponent);
			BillboardComponentInst->SetHiddenInGame(true);
		}
	}
#endif
}

void ATPSPlayerStart::Deactivate()
{
	bIsActive = false;

	UE_LOG(LogTemp, Log, TEXT("[PlayerStart] Deactivated: %s (%s)"),
		*GetName(), *DisplayName.ToString());
}

void ATPSPlayerStart::Activate()
{
	bIsActive = true;

	UE_LOG(LogTemp, Log, TEXT("[PlayerStart] Activated: %s (%s)"),
		*GetName(), *DisplayName.ToString());
}
