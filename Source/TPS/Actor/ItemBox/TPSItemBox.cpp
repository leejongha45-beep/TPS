#include "Actor/ItemBox/TPSItemBox.h"
#include "Component/Action/TPSPlayerInteractionComponent.h"
#include "Component/Action/TPSItemBoxInteractionComponent.h"
#include "Pawn/Character/Player/TPSPlayer.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

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

			BoxCollisionInst->OnComponentBeginOverlap.AddDynamic(this, &ATPSItemBox::OnBoxBeginOverlap);
			BoxCollisionInst->OnComponentEndOverlap.AddDynamic(this, &ATPSItemBox::OnBoxEndOverlap);
		}
	}

	if (!InteractionComponentInst)
	{
		InteractionComponentInst = CreateDefaultSubobject<UTPSItemBoxInteractionComponent>(TEXT("InteractionComponent"));
		ensure(InteractionComponentInst);
	}
}

void ATPSItemBox::Interact()
{
	if (ensure(InteractionComponentInst))
	{
		InteractionComponentInst->ToggleItemBox();
	}
}

void ATPSItemBox::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	ATPSPlayer* pPlayer = Cast<ATPSPlayer>(OtherActor);
	if (!pPlayer) return;

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

	UE_LOG(LogItemBox, Log, TEXT("[OnBoxEndOverlap] Player left trigger zone"));
}
