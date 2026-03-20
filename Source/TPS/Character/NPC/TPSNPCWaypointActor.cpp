#include "Character/NPC/TPSNPCWaypointActor.h"

#if WITH_EDITORONLY_DATA
#include "Components/BillboardComponent.h"
#endif

ATPSNPCWaypointActor::ATPSNPCWaypointActor()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* pRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	if (ensure(pRoot))
	{
		SetRootComponent(pRoot);
	}

#if WITH_EDITORONLY_DATA
	SpriteComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (ensure(SpriteComponent))
	{
		SpriteComponent->SetupAttachment(pRoot);
		SpriteComponent->SetHiddenInGame(true);
	}
#endif
}
