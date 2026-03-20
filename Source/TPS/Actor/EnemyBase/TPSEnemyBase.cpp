#include "Actor/EnemyBase/TPSEnemyBase.h"
#include "Character/NPC/TPSNPCWaypointActor.h"
#include "Core/Subsystem/TPSTargetSubsystem.h"
#include "Engine/World.h"

ATPSEnemyBase::ATPSEnemyBase()
{
}

void ATPSEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	CollectWaypoints();

	if (UTPSTargetSubsystem* TargetSS = GetWorld()->GetSubsystem<UTPSTargetSubsystem>())
	{
		TargetSS->RegisterEnemyBase(this);
	}

	UE_LOG(LogTemp, Log, TEXT("[EnemyBase] %s initialized — HP: %.0f / %.0f, Waypoints: %d"),
		*GetName(), GetHealth(), GetMaxHealth(), CachedWaypoints.Num());
}

void ATPSEnemyBase::OnBaseDestroyed()
{
	if (UTPSTargetSubsystem* TargetSS = GetWorld()->GetSubsystem<UTPSTargetSubsystem>())
	{
		TargetSS->UnregisterEnemyBase(this);
	}

	UE_LOG(LogTemp, Warning, TEXT("[EnemyBase] %s destroyed!"), *GetName());

	Super::OnBaseDestroyed();
}

void ATPSEnemyBase::CollectWaypoints()
{
	CachedWaypoints.Reset();

	ATPSNPCWaypointActor* pCurrent = NPCWaypointStart.Get();
	while (pCurrent)
	{
		CachedWaypoints.Add(pCurrent->GetActorLocation());
		WaypointAcceptRadius = pCurrent->AcceptRadius;
		pCurrent = pCurrent->NextWaypoint.Get();
	}

	// 마지막에 기지 위치 추가
	CachedWaypoints.Add(GetActorLocation());
}
