// TPSEnemySpawnPoint.cpp

#include "Wave/TPSEnemySpawnPoint.h"

#if WITH_EDITORONLY_DATA
#include "Components/BillboardComponent.h"
#endif

ATPSEnemySpawnPoint::ATPSEnemySpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	// 루트 — SceneComponent
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