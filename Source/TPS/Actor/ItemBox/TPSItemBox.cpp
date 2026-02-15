#include "Actor/ItemBox/TPSItemBox.h"
#include "Actor/ItemBox/TPSItemBoxInteractionComponent.h"
#include "Component/Action/TPSPlayerInteractionComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Pawn/Character/Player/TPSPlayer.h"

DEFINE_LOG_CATEGORY(LogItemBox);

ATPSItemBox::ATPSItemBox()
{
	PrimaryActorTick.bCanEverTick = false;

	if (!MeshInst)
	{
		MeshInst = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
		if (ensure(MeshInst))
		{
			SetRootComponent(MeshInst);
		}
	}

	if (!BoxCollisionInst)
	{
		BoxCollisionInst = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
		if (ensure(BoxCollisionInst))
		{
			BoxCollisionInst->SetupAttachment(RootComponent);
			BoxCollisionInst->SetCollisionProfileName(TEXT("Trigger"));
			BoxCollisionInst->SetBoxExtent(FVector(150.f, 150.f, 100.f));

			if (!BoxCollisionInst->OnComponentBeginOverlap.IsBound())
			{
				BoxCollisionInst->OnComponentBeginOverlap.AddDynamic(this, &ATPSItemBox::OnBoxBeginOverlap);
			}

			if (!BoxCollisionInst->OnComponentEndOverlap.IsBound())
			{
				BoxCollisionInst->OnComponentEndOverlap.AddDynamic(this, &ATPSItemBox::OnBoxEndOverlap);
			}
		}
	}

	if (!InteractionComponentInst)
	{
		InteractionComponentInst = CreateDefaultSubobject<UTPSItemBoxInteractionComponent>(TEXT("InteractionComponent"));
		ensure(InteractionComponentInst);
	}
}

void ATPSItemBox::SpawnItem(EItemType TargetItem)
{
	int32 ItemIndex = static_cast<int32>(TargetItem);

	if (ensure(ItemClassArray.IsValidIndex(ItemIndex)))
	{
		if (!ensure(ItemClassArray[ItemIndex])) return;

		if (!SpawnedWeapon)
		{
			SpawnedWeapon = GetWorld()->SpawnActor<AActor>(ItemClassArray[ItemIndex]);
			ensure(SpawnedWeapon);
		}
	}
}

void ATPSItemBox::Interact()
{
	if (ensure(InteractionComponentInst))
	{
		if (ensure(InteractableInterface))
		{
			ATPSPlayer* pPlayer = Cast<ATPSPlayer>(InteractableInterface.GetObject());
			if (ensure(pPlayer))
			{
				APlayerController* pController = Cast<APlayerController>(pPlayer->GetController());
				InteractionComponentInst->ToggleItemBox(!InteractionComponentInst->GetIsOpen(), pController);
			}
		}
	}
}

void ATPSItemBox::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                    bool bFromSweep, const FHitResult& SweepResult)
{
	ATPSPlayer* pPlayer = Cast<ATPSPlayer>(OtherActor);
	if (!ensure(pPlayer)) return;

	InteractableInterface = pPlayer;
	ensure(InteractableInterface);

	UTPSPlayerInteractionComponent* pInteractionComponent = pPlayer->GetInteractionComponent();
	if (ensure(pInteractionComponent))
	{
		pInteractionComponent->SetCurrentTarget(this);
	}

	UE_LOG(LogItemBox, Log, TEXT("[OnBoxBeginOverlap] Player entered trigger zone"));
}

void ATPSItemBox::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ATPSPlayer* pPlayer = Cast<ATPSPlayer>(OtherActor);
	if (!pPlayer) return;

	UTPSPlayerInteractionComponent* pInteractionComponent = pPlayer->GetInteractionComponent();
	if (ensure(pInteractionComponent))
	{
		pInteractionComponent->ClearCurrentTarget(this);
	}

	InteractableInterface = nullptr;
	ensure(!InteractableInterface);

	UE_LOG(LogItemBox, Log, TEXT("[OnBoxEndOverlap] Player left trigger zone"));
}