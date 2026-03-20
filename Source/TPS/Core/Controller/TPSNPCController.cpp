#include "Core/Controller/TPSNPCController.h"
#include "Utils/Interface/Action/Fireable.h"
#include "ECS/Scheduler/EnemyManagerSubsystem.h"
#include "ECS/Scheduler/EnemyScheduler.h"
#include "ECS/Component/Components.h"
#include "Character/NPC/TPSNPCWaypointActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"

DECLARE_STATS_GROUP(TEXT("NPCController"), STATGROUP_NPCController, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("AI_Control_Tick"), STAT_NPCControlTick, STATGROUP_NPCController);
DECLARE_CYCLE_STAT(TEXT("FindNearestEnemy"), STAT_NPCFindEnemy, STATGROUP_NPCController);

ATPSNPCController::ATPSNPCController()
{
	PrimaryActorTick.bCanEverTick = false;
	AIControlTick.bCanEverTick = true;
}

void ATPSNPCController::RegisterActorTickFunctions(bool bRegister)
{
	Super::RegisterActorTickFunctions(bRegister);

	if (bRegister)
	{
		AIControlTick.Target = this;
		AIControlTick.RegisterTickFunction(GetLevel());
		AIControlTick.SetTickFunctionEnable(true);
	}
	else
	{
		if (AIControlTick.IsTickFunctionRegistered())
		{
			AIControlTick.UnRegisterTickFunction();
		}
	}
}

void ATPSNPCController::BeginPlay()
{
	Super::BeginPlay();
	CollectWaypoints();
}

void ATPSNPCController::ResetAIState()
{
	CurrentWaypointIndex = 0;
	bIsMovingToWaypoint = false;
	bHasCachedEnemy = false;
	CachedEnemyLocation = FVector::ZeroVector;
	AIFrameCounter = 0;
}

void ATPSNPCController::AI_Control_Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_NPCControlTick);

	APawn* pPawn = GetPawn();
	if (!pPawn) { return; }

	// 1. 적 감지 — N프레임마다 갱신
	if (AIFrameCounter % DetectionInterval == 0)
	{
		bHasCachedEnemy = FindNearestEnemy(CachedEnemyLocation);
	}
	++AIFrameCounter;

	if (bHasCachedEnemy)
	{
		const float DistSq = FVector::DistSquared(pPawn->GetActorLocation(), CachedEnemyLocation);
		const float FireRangeSq = FireRange * FireRange;

		if (DistSq <= FireRangeSq)
		{
			// 사거리 내 → 정지 + CMC 비활성화 + 회전 + 사격
			if (bIsMovingToWaypoint)
			{
				StopMovement();
				bIsMovingToWaypoint = false;
			}

			// CMC 틱 비활성화 — 정지 상태에서 불필요한 물리 연산 제거
			if (auto* CMC = pPawn->FindComponentByClass<UCharacterMovementComponent>())
			{
				if (CMC->IsComponentTickEnabled())
				{
					CMC->SetComponentTickEnabled(false);
				}
			}

			RotateToTarget(CachedEnemyLocation, DeltaTime);

			CommandReload();
			CommandFire();
			return;
		}
	}

	// 적 없거나 사거리 밖 → 사격 중지 + 재장전
	CommandCeaseFire();
	CommandReload();

	// CMC 틱 재활성화 — 이동 필요
	if (auto* CMC = pPawn->FindComponentByClass<UCharacterMovementComponent>())
	{
		if (!CMC->IsComponentTickEnabled())
		{
			CMC->SetComponentTickEnabled(true);
		}
	}

	// 2. 웨이포인트 러쉬
	if (CachedWaypoints.IsValidIndex(CurrentWaypointIndex))
	{
		const FVector& WPLocation = CachedWaypoints[CurrentWaypointIndex];
		const float DistToWP = FVector::DistSquared2D(pPawn->GetActorLocation(), WPLocation);
		const float AcceptSq = WaypointAcceptRadius * WaypointAcceptRadius;

		if (DistToWP <= AcceptSq)
		{
			// 도착 → 다음 웨이포인트
			++CurrentWaypointIndex;
			if (CachedWaypoints.IsValidIndex(CurrentWaypointIndex))
			{
				MoveToLocation(CachedWaypoints[CurrentWaypointIndex]);
				bIsMovingToWaypoint = true;
			}
			else
			{
				// 마지막 웨이포인트 도착
				StopMovement();
				bIsMovingToWaypoint = false;
			}
		}
		else if (!bIsMovingToWaypoint)
		{
			MoveToLocation(WPLocation);
			bIsMovingToWaypoint = true;
		}

		// 이동 중 → 이동 방향으로 회전
		if (bIsMovingToWaypoint)
		{
			RotateToTarget(WPLocation, DeltaTime);
		}
	}
}

void ATPSNPCController::CollectWaypoints()
{
	UWorld* pWorld = GetWorld();
	if (!pWorld) { return; }

	CachedWaypoints.Reset();

	// Head 찾기 — 다른 웨이포인트가 가리키지 않는 것
	TSet<ATPSNPCWaypointActor*> AllWaypoints;
	TSet<ATPSNPCWaypointActor*> ReferencedWaypoints;

	for (TActorIterator<ATPSNPCWaypointActor> It(pWorld); It; ++It)
	{
		ATPSNPCWaypointActor* WP = *It;
		AllWaypoints.Add(WP);
		if (WP->NextWaypoint)
		{
			ReferencedWaypoints.Add(WP->NextWaypoint);
		}
	}

	ATPSNPCWaypointActor* Head = nullptr;
	for (ATPSNPCWaypointActor* WP : AllWaypoints)
	{
		if (!ReferencedWaypoints.Contains(WP))
		{
			Head = WP;
			break;
		}
	}

	if (!Head) { return; }

	WaypointAcceptRadius = Head->AcceptRadius;
	ATPSNPCWaypointActor* Current = Head;
	int32 SafetyCount = 0;
	constexpr int32 MaxWaypoints = 100;

	while (Current && SafetyCount < MaxWaypoints)
	{
		CachedWaypoints.Add(Current->GetActorLocation());
		Current = Current->NextWaypoint;
		++SafetyCount;
	}

}

bool ATPSNPCController::FindNearestEnemy(FVector& OutLocation) const
{
	SCOPE_CYCLE_COUNTER(STAT_NPCFindEnemy);
	const APawn* pPawn = GetPawn();
	if (!pPawn) { return false; }

	const UWorld* pWorld = GetWorld();
	if (!pWorld) { return false; }

	const UEnemyManagerSubsystem* pEnemyMgr = pWorld->GetSubsystem<UEnemyManagerSubsystem>();
	if (!pEnemyMgr || !pEnemyMgr->GetScheduler()) { return false; }

	entt::registry& Registry = pEnemyMgr->GetScheduler()->GetRegistry();
	const FVector MyLocation = pPawn->GetActorLocation();
	const float DetectionRadiusSq = DetectionRadius * DetectionRadius;

	float BestDistSq = DetectionRadiusSq;
	bool bFound = false;

	auto View = Registry.view<CTransformPrev, CEnemyStatePrev>();
	for (auto Entity : View)
	{
		const EEnemyState State = View.get<CEnemyStatePrev>(Entity).State;
		if (State == EEnemyState::Dying || State == EEnemyState::Dead) { continue; }

		const FVector& EnemyPos = View.get<CTransformPrev>(Entity).Position;
		const float DistSq = FVector::DistSquared(MyLocation, EnemyPos);

		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			OutLocation = EnemyPos;
			bFound = true;
		}
	}

	return bFound;
}

void ATPSNPCController::RotateToTarget(const FVector& TargetLocation, float DeltaTime)
{
	const APawn* pPawn = GetPawn();
	if (!pPawn) { return; }

	const FVector Direction = TargetLocation - pPawn->GetActorLocation();
	const FRotator TargetRot = Direction.Rotation();
	const FRotator CurrentRot = GetControlRotation();
	const FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, AimInterpSpeed);

	SetControlRotation(NewRot);
}

void ATPSNPCController::CommandFire()
{
	if (auto* pFireable = Cast<IFireable>(GetPawn()))
	{
		pFireable->StartFire();
	}
}

void ATPSNPCController::CommandCeaseFire()
{
	if (auto* pFireable = Cast<IFireable>(GetPawn()))
	{
		pFireable->StopFire();
	}
}

void ATPSNPCController::CommandReload()
{
	if (auto* pFireable = Cast<IFireable>(GetPawn()))
	{
		pFireable->Reload();
	}
}
