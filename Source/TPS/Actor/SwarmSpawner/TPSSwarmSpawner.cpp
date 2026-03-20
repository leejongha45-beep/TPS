#include "Actor/SwarmSpawner/TPSSwarmSpawner.h"
#include "Components/SceneComponent.h"
#include "Components/BillboardComponent.h"

ATPSSwarmSpawner::ATPSSwarmSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

#if WITH_EDITORONLY_DATA
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (SpriteComponent)
	{
		SpriteComponent->SetupAttachment(SceneRoot);
	}
#endif
}

void ATPSSwarmSpawner::StartSpawning()
{
	bIsActive = true;
	LastSpawnTime = -SpawnInterval; // 시작 즉시 첫 스폰
}

void ATPSSwarmSpawner::StopSpawning()
{
	bIsActive = false;
}

FSwarm ATPSSwarmSpawner::SpawnSwarm() const
{
	return SpawnSwarm(TroopCount);
}

FSwarm ATPSSwarmSpawner::SpawnSwarm(int32 OverrideTroopCount) const
{
	FSwarm Swarm;
	Swarm.Position = CalcSpawnPosition();
	Swarm.Team = Team;
	Swarm.TroopCount = OverrideTroopCount;
	Swarm.UnitMaxHP = UnitMaxHP;
	Swarm.CurrentUnitHP = UnitMaxHP;
	Swarm.AttackPower = AttackPower;
	Swarm.MoveSpeed = MoveSpeed;
	Swarm.RouteIndex = RouteIndex;
	return Swarm;
}

FVector ATPSSwarmSpawner::CalcSpawnPosition() const
{
	FVector Pos = GetActorLocation();

	if (SpawnRadius > 0.f)
	{
		const float Angle = FMath::FRandRange(0.f, 2.f * PI);
		const float RandR = FMath::FRandRange(0.f, SpawnRadius);
		Pos += FVector(FMath::Cos(Angle) * RandR, FMath::Sin(Angle) * RandR, 0.f);
	}

	return Pos;
}
