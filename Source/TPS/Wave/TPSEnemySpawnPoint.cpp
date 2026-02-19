#include "Wave/TPSEnemySpawnPoint.h"
#include "Components/BillboardComponent.h"
#include "Components/SphereComponent.h"

ATPSEnemySpawnPoint::ATPSEnemySpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	// ① 루트 컴포넌트 (씬 루트)
	USceneComponent* pRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	if (ensure(pRoot))
	{
		SetRootComponent(pRoot);
	}

#if WITH_EDITORONLY_DATA
	// ② 빌보드 — 에디터에서 아이콘 표시
	if (!BillboardComponentInst)
	{
		BillboardComponentInst = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
		if (ensure(BillboardComponentInst))
		{
			BillboardComponentInst->SetupAttachment(RootComponent);
			BillboardComponentInst->SetHiddenInGame(true);
		}
	}

	// ③ SpawnRadius 시각화용 구체
	if (!DebugRadiusSphereInst)
	{
		DebugRadiusSphereInst = CreateDefaultSubobject<USphereComponent>(TEXT("DebugRadiusSphere"));
		if (ensure(DebugRadiusSphereInst))
		{
			DebugRadiusSphereInst->SetupAttachment(RootComponent);
			DebugRadiusSphereInst->SetSphereRadius(SpawnRadius);
			DebugRadiusSphereInst->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			DebugRadiusSphereInst->SetHiddenInGame(true);
			DebugRadiusSphereInst->ShapeColor = FColor::Orange;
		}
	}
#endif
}

FVector ATPSEnemySpawnPoint::GetRandomLocationInRadius() const
{
	// ① 원형 랜덤 분산 (XY만)
	const float Angle = FMath::RandRange(0.f, 2.f * PI);
	const float Dist = FMath::RandRange(0.f, SpawnRadius);

	return GetActorLocation() + FVector(
		FMath::Cos(Angle) * Dist,
		FMath::Sin(Angle) * Dist,
		0.f
	);
}

void ATPSEnemySpawnPoint::Deactivate()
{
	bIsActive = false;

	UE_LOG(LogTemp, Log, TEXT("[SpawnPoint] Deactivated: %s"), *GetName());
}

void ATPSEnemySpawnPoint::Activate()
{
	bIsActive = true;

	UE_LOG(LogTemp, Log, TEXT("[SpawnPoint] Activated: %s"), *GetName());
}
