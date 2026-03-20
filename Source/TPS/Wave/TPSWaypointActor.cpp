// TPSWaypointActor.cpp

#include "Wave/TPSWaypointActor.h"

#if WITH_EDITORONLY_DATA
#include "Components/BillboardComponent.h"
#endif

ATPSWaypointActor::ATPSWaypointActor()
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
