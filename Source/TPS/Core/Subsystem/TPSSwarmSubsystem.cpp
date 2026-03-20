#include "Core/Subsystem/TPSSwarmSubsystem.h"
#include "Core/Subsystem/TPSTargetSubsystem.h"
#include "ECS/Scheduler/EnemyManagerSubsystem.h"
#include "ECS/Data/EnemySpawnParams.h"
#include "ECS/Data/TPSEnemyTypeDataAsset.h"
#include "ECS/Component/Components.h"
#include "ECS/Scheduler/EnemyScheduler.h"
#include "Actor/AllyBase/TPSAllyBase.h"
#include "Actor/EnemyBase/TPSEnemyBase.h"
#include "Actor/SwarmSpawner/TPSSwarmSpawner.h"
#include "Wave/TPSWaypointActor.h"
#include "Wave/TPSWaveSettings.h"
#include "EngineUtils.h"
#include "Character/NPC/TPSNPCWaypointActor.h"
#include "Character/NPC/TPSSoldierNPC.h"
#include "Core/Subsystem/TPSNPCPoolSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "UI/ViewModel/MinimapViewModel.h"

void UTPSSwarmSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CachedSettings = GetDefault<UTPSWaveSettings>();
	if (CachedSettings.Get())
	{
		CachedEnemyType = CachedSettings->EnemyType.LoadSynchronous();
	}

	// 미니맵 뷰모델 생성
	MinimapViewModelInst = NewObject<UMinimapViewModel>(this);
}

TStatId UTPSSwarmSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UTPSSwarmSubsystem, STATGROUP_Tickables);
}

void UTPSSwarmSubsystem::Tick(float DeltaTime)
{
	if (!bWaypointsCollected)
	{
		CollectWaypoints();
		bWaypointsCollected = true;
	}

	// 0. 웨이브 스폰 (군집 생성)
	TickWaveSpawning(DeltaTime);

	if (Swarms.Num() == 0) { return; }

	// 1. 군집 이동
	MoveSwarms(DeltaTime);

	// 2. 군집 vs 군집 전투
	ProcessSwarmCombat(DeltaTime);

	// 3. 군집 vs 기지
	ProcessSwarmVsBase(DeltaTime);

	// 4. Unfold된 군집 위치 갱신
	UpdateUnfoldedSwarmPositions();

	// 5. 플레이어 근접 감지 + Unfold/Fold
	CheckPlayerProximity();

	// 6. 병력 0인 군집 제거 (역순)
	for (int32 i = Swarms.Num() - 1; i >= 0; --i)
	{
		if (Swarms[i].TroopCount <= 0 && !Swarms[i].bUnfolded)
		{
			Swarms.RemoveAtSwap(i);
		}
	}

	// 7. 미니맵 뷰모델 갱신
	if (MinimapViewModelInst.Get())
	{
		MinimapViewModelInst->Update(GetWorld(), FVector2D::ZeroVector, 50000.f);
	}
}

// ──────────── 군집 관리 ────────────

void UTPSSwarmSubsystem::AddSwarm(const FSwarm& InSwarm)
{
	FSwarm NewSwarm = InSwarm;
	NewSwarm.SwarmID = NextSwarmID++;
	Swarms.Add(NewSwarm);
}

void UTPSSwarmSubsystem::RemoveSwarm(int32 Index)
{
	if (Swarms.IsValidIndex(Index))
	{
		Swarms.RemoveAtSwap(Index);
	}
}

// ──────────── 웨이포인트 수집 ────────────

void UTPSSwarmSubsystem::CollectWaypoints()
{
	UWorld* World = GetWorld();
	if (!World) { return; }

	// 적 웨이포인트 — RouteIndex별 그룹핑, 링크드리스트 순서 유지
	{
		TMap<int32, class ATPSWaypointActor*> HeadByRoute;
		TSet<class ATPSWaypointActor*> Referenced;

		for (TActorIterator<ATPSWaypointActor> It(World); It; ++It)
		{
			if ((*It)->NextWaypoint) { Referenced.Add((*It)->NextWaypoint); }
		}

		for (TActorIterator<ATPSWaypointActor> It(World); It; ++It)
		{
			if (!Referenced.Contains(*It))
			{
				HeadByRoute.Add((*It)->RouteIndex, *It);
			}
		}

		// RouteIndex 순서로 정렬하여 배열화
		HeadByRoute.KeySort([](int32 A, int32 B) { return A < B; });

		for (auto& [RouteIdx, Head] : HeadByRoute)
		{
			EnemyWaypointAcceptRadius = Head->AcceptRadius;
			TArray<FVector> Route;
			class ATPSWaypointActor* Current = Head;
			int32 Safety = 0;
			while (Current && Safety < 100)
			{
				Route.Add(Current->GetActorLocation());
				Current = Current->NextWaypoint;
				++Safety;
			}

			// RouteIndex가 배열 인덱스와 일치하도록 패딩
			while (EnemyWaypointRoutes.Num() <= RouteIdx)
			{
				EnemyWaypointRoutes.Add(TArray<FVector>());
			}
			EnemyWaypointRoutes[RouteIdx] = MoveTemp(Route);
		}
	}

	// 아군 웨이포인트 — RouteIndex별 그룹핑
	{
		TMap<int32, class ATPSNPCWaypointActor*> HeadByRoute;
		TSet<class ATPSNPCWaypointActor*> Referenced;

		for (TActorIterator<ATPSNPCWaypointActor> It(World); It; ++It)
		{
			if ((*It)->NextWaypoint) { Referenced.Add((*It)->NextWaypoint); }
		}

		for (TActorIterator<ATPSNPCWaypointActor> It(World); It; ++It)
		{
			if (!Referenced.Contains(*It))
			{
				HeadByRoute.Add((*It)->RouteIndex, *It);
			}
		}

		HeadByRoute.KeySort([](int32 A, int32 B) { return A < B; });

		for (auto& [RouteIdx, Head] : HeadByRoute)
		{
			AllyWaypointAcceptRadius = Head->AcceptRadius;
			TArray<FVector> Route;
			class ATPSNPCWaypointActor* Current = Head;
			int32 Safety = 0;
			while (Current && Safety < 100)
			{
				Route.Add(Current->GetActorLocation());
				Current = Current->NextWaypoint;
				++Safety;
			}

			while (AllyWaypointRoutes.Num() <= RouteIdx)
			{
				AllyWaypointRoutes.Add(TArray<FVector>());
			}
			AllyWaypointRoutes[RouteIdx] = MoveTemp(Route);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[Swarm] CollectWaypoints — EnemyRoutes=%d, AllyRoutes=%d"),
		EnemyWaypointRoutes.Num(), AllyWaypointRoutes.Num());
}

// ──────────── 1. 군집 이동 ────────────

void UTPSSwarmSubsystem::MoveSwarms(float DeltaTime)
{
	for (FSwarm& Swarm : Swarms)
	{
		if (Swarm.bUnfolded || Swarm.bEngaged) { continue; }
		if (Swarm.TroopCount <= 0) { continue; }

		const TArray<TArray<FVector>>& Routes = (Swarm.Team == ESwarmTeam::Enemy)
			? EnemyWaypointRoutes : AllyWaypointRoutes;
		const float AcceptRadius = (Swarm.Team == ESwarmTeam::Enemy)
			? EnemyWaypointAcceptRadius : AllyWaypointAcceptRadius;

		if (!Routes.IsValidIndex(Swarm.RouteIndex)) { continue; }
		const TArray<FVector>& Waypoints = Routes[Swarm.RouteIndex];

		if (!Waypoints.IsValidIndex(Swarm.WaypointIndex)) { continue; }

		const FVector& Target = Waypoints[Swarm.WaypointIndex];
		const FVector Direction = Target - Swarm.Position;
		const float DistSq = Direction.SizeSquared2D();
		const float AcceptSq = AcceptRadius * AcceptRadius;

		if (DistSq <= AcceptSq)
		{
			++Swarm.WaypointIndex;
		}
		else
		{
			const FVector MoveDir = Direction.GetSafeNormal2D();
			Swarm.Position += MoveDir * Swarm.MoveSpeed * DeltaTime;
		}
	}
}

// ──────────── 2. 군집 vs 군집 전투 ────────────

void UTPSSwarmSubsystem::ProcessSwarmCombat(float DeltaTime)
{
	const float EngageRadiusSq = EngageRadius * EngageRadius;

	// 일괄 리셋 — 루프 안에서 리셋하면 이전 페어 결과가 덮어씌워짐
	for (FSwarm& S : Swarms)
	{
		S.bEngaged = false;
	}

	for (int32 i = 0; i < Swarms.Num(); ++i)
	{
		FSwarm& A = Swarms[i];
		if (A.bUnfolded || A.TroopCount <= 0) { continue; }

		for (int32 j = i + 1; j < Swarms.Num(); ++j)
		{
			FSwarm& B = Swarms[j];
			if (B.bUnfolded || B.TroopCount <= 0) { continue; }
			if (A.Team == B.Team) { continue; }

			const float DistSq = FVector::DistSquared(A.Position, B.Position);
			if (DistSq > EngageRadiusSq) { continue; }

			// 교전 중
			A.bEngaged = true;
			B.bEngaged = true;

			// 서로 HP 감산 — 병력수 가중치 적용
			A.CurrentUnitHP -= B.AttackPower * B.TroopCount * DeltaTime;
			B.CurrentUnitHP -= A.AttackPower * A.TroopCount * DeltaTime;

			// A 개체 사망 체크
			if (A.CurrentUnitHP <= 0.f)
			{
				--A.TroopCount;
				A.CurrentUnitHP = A.UnitMaxHP;
			}

			// B 개체 사망 체크
			if (B.CurrentUnitHP <= 0.f)
			{
				--B.TroopCount;
				B.CurrentUnitHP = B.UnitMaxHP;
			}
		}
	}
}

// ──────────── 3. 군집 vs 기지 ────────────

void UTPSSwarmSubsystem::ProcessSwarmVsBase(float DeltaTime)
{
	UTPSTargetSubsystem* TargetSS = GetWorld()->GetSubsystem<UTPSTargetSubsystem>();
	if (!TargetSS) { return; }

	const float BaseRadiusSq = BaseAttackRadius * BaseAttackRadius;

	for (FSwarm& Swarm : Swarms)
	{
		if (Swarm.bUnfolded || Swarm.TroopCount <= 0) { continue; }

		if (Swarm.Team == ESwarmTeam::Enemy)
		{
			// 적군집 → 아군 기지 공격
			for (const auto& Base : TargetSS->GetAllyBases())
			{
				if (!Base.Get() || Base->IsDestroyed()) { continue; }

				const float DistSq = FVector::DistSquared(Swarm.Position, Base->GetActorLocation());
				if (DistSq <= BaseRadiusSq)
				{
					UGameplayStatics::ApplyDamage(Base.Get(), Swarm.AttackPower * DeltaTime, nullptr, nullptr, nullptr);
				}
			}
		}
		else
		{
			// 아군군집 → 적 기지 공격
			for (const auto& Base : TargetSS->GetEnemyBases())
			{
				if (!Base.Get() || Base->IsDestroyed()) { continue; }

				const float DistSq = FVector::DistSquared(Swarm.Position, Base->GetActorLocation());
				if (DistSq <= BaseRadiusSq)
				{
					UGameplayStatics::ApplyDamage(Base.Get(), Swarm.AttackPower * DeltaTime, nullptr, nullptr, nullptr);
				}
			}
		}
	}
}

// ──────────── 4. Unfold된 군집 위치 갱신 ────────────

void UTPSSwarmSubsystem::UpdateUnfoldedSwarmPositions()
{
	UWorld* World = GetWorld();
	if (!World) { return; }

	UTPSTargetSubsystem* TargetSS = World->GetSubsystem<UTPSTargetSubsystem>();
	UEnemyManagerSubsystem* EnemyMgr = World->GetSubsystem<UEnemyManagerSubsystem>();

	for (FSwarm& Swarm : Swarms)
	{
		if (!Swarm.bUnfolded) { continue; }

		if (Swarm.Team == ESwarmTeam::Enemy && EnemyMgr && EnemyMgr->GetScheduler())
		{
			// 첫 번째 생존 엔티티 위치로 갱신
			entt::registry& Registry = EnemyMgr->GetScheduler()->GetRegistry();

			auto View = Registry.view<CTransform, CEnemyState>();
			for (auto Entity : View)
			{
				const auto& State = View.get<CEnemyState>(Entity);
				if (State.State == EEnemyState::Dying || State.State == EEnemyState::Dead) { continue; }

				Swarm.Position = View.get<CTransform>(Entity).Position;
				break;
			}
		}
		else if (Swarm.Team == ESwarmTeam::Ally && TargetSS)
		{
			// 같은 SwarmID의 첫 번째 NPC 위치로 갱신
			for (const auto& NPCActor : TargetSS->GetNPCs())
			{
				class ATPSSoldierNPC* NPC = Cast<ATPSSoldierNPC>(NPCActor.Get());
				if (NPC && NPC->OwnerSwarmID == Swarm.SwarmID)
				{
					Swarm.Position = NPC->GetActorLocation();
					break;
				}
			}
		}
	}
}

// ──────────── 5. 플레이어 근접 감지 ────────────

void UTPSSwarmSubsystem::CheckPlayerProximity()
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player) { return; }

	const FVector PlayerPos = Player->GetActorLocation();
	const float UnfoldRadiusSq = UnfoldRadius * UnfoldRadius;
	const float FoldRadiusSq = FoldRadius * FoldRadius;

	for (int32 i = 0; i < Swarms.Num(); ++i)
	{
		FSwarm& Swarm = Swarms[i];
		if (Swarm.TroopCount <= 0) { continue; }

		const float DistSq = FVector::DistSquared(PlayerPos, Swarm.Position);

		if (!Swarm.bUnfolded && DistSq <= UnfoldRadiusSq)
		{
			UnfoldSwarm(i);
		}
		else if (Swarm.bUnfolded && DistSq > FoldRadiusSq)
		{
			FoldSwarm(i);
		}
	}
}

// ──────────── Unfold — 군집 → 엔티티/NPC ────────────

void UTPSSwarmSubsystem::UnfoldSwarm(int32 Index)
{
	if (!Swarms.IsValidIndex(Index)) { return; }

	FSwarm& Swarm = Swarms[Index];
	UWorld* World = GetWorld();

	if (Swarm.Team == ESwarmTeam::Enemy)
	{
		// ECS 엔티티 스폰
		UEnemyManagerSubsystem* EnemyMgr = World->GetSubsystem<UEnemyManagerSubsystem>();
		if (!EnemyMgr) { return; }

		for (int32 i = 0; i < Swarm.TroopCount; ++i)
		{
			FEnemySpawnParams Params;
			Params.Position = Swarm.Position + FVector(
				FMath::FRandRange(-500.f, 500.f),
				FMath::FRandRange(-500.f, 500.f),
				0.f
			);
			Params.MaxHealth = Swarm.UnitMaxHP;
			Params.AttackDamage = Swarm.AttackPower;
			if (CachedEnemyType.Get())
			{
				Params.MaxSpeed = CachedEnemyType->MaxSpeed;
				Params.AttackCooldown = CachedEnemyType->AttackCooldown;
				Params.MeshYawOffset = CachedEnemyType->MeshYawOffset;
			}

			EnemyMgr->QueueSpawn(Params);
		}
	}
	else
	{
		// NPC 풀에서 가져와서 활성화
		UTPSNPCPoolSubsystem* NPCPool = World->GetSubsystem<UTPSNPCPoolSubsystem>();
		if (!NPCPool) { return; }

		for (int32 i = 0; i < Swarm.TroopCount; ++i)
		{
			ATPSSoldierNPC* NPC = NPCPool->GetNPC();
			if (NPC)
			{
				NPC->OwnerSwarmID = Swarm.SwarmID;
				const FVector SpawnPos = Swarm.Position + FVector(
					FMath::FRandRange(-500.f, 500.f),
					FMath::FRandRange(-500.f, 500.f),
					0.f
				);
				NPCPool->ActivateNPC(NPC, SpawnPos);
			}
		}
	}

	Swarm.bUnfolded = true;
}

// ──────────── Fold — 엔티티/NPC → 군집 ────────────

void UTPSSwarmSubsystem::FoldSwarm(int32 Index)
{
	if (!Swarms.IsValidIndex(Index)) { return; }

	FSwarm& Swarm = Swarms[Index];
	UWorld* World = GetWorld();
	const float CollectRadiusSq = FoldRadius * FoldRadius;

	if (Swarm.Team == ESwarmTeam::Enemy)
	{
		// ECS 엔티티 수거 — FoldRadius 내 생존 엔티티 카운트 후 Dead 처리
		UEnemyManagerSubsystem* EnemyMgr = World->GetSubsystem<UEnemyManagerSubsystem>();
		if (EnemyMgr && EnemyMgr->GetScheduler())
		{
			entt::registry& Registry = EnemyMgr->GetScheduler()->GetRegistry();
			int32 AliveCount = 0;

			auto View = Registry.view<CTransform, CEnemyState, CHealth>();
			for (auto Entity : View)
			{
				auto& State = View.get<CEnemyState>(Entity);
				if (State.State == EEnemyState::Dying || State.State == EEnemyState::Dead) { continue; }

				auto& Transform = View.get<CTransform>(Entity);
				const float DistSq = FVector::DistSquared(Transform.Position, Swarm.Position);
				if (DistSq <= CollectRadiusSq)
				{
					++AliveCount;
					State.State = EEnemyState::Dead;
				}
			}

			Swarm.TroopCount = AliveCount;
			UE_LOG(LogTemp, Log, TEXT("[Swarm] Fold Enemy swarm at %s — %d troops recovered"),
				*Swarm.Position.ToString(), AliveCount);
		}
	}
	else
	{
		// NPC 수거 — SwarmID가 일치하는 NPC만 풀 반환
		UTPSTargetSubsystem* TargetSS = World->GetSubsystem<UTPSTargetSubsystem>();
		UTPSNPCPoolSubsystem* NPCPool = World->GetSubsystem<UTPSNPCPoolSubsystem>();
		if (TargetSS && NPCPool)
		{
			TArray<class ATPSSoldierNPC*> ToReturn;

			for (const auto& NPCActor : TargetSS->GetNPCs())
			{
				class ATPSSoldierNPC* NPC = Cast<ATPSSoldierNPC>(NPCActor.Get());
				if (NPC && NPC->OwnerSwarmID == Swarm.SwarmID)
				{
					ToReturn.Add(NPC);
				}
			}

			Swarm.TroopCount = ToReturn.Num();

			for (class ATPSSoldierNPC* NPC : ToReturn)
			{
				NPC->OwnerSwarmID = INDEX_NONE;
				NPCPool->ReturnNPC(NPC);
			}

			UE_LOG(LogTemp, Log, TEXT("[Swarm] Fold Ally swarm ID=%d — %d troops recovered"), Swarm.SwarmID, Swarm.TroopCount);
		}
	}

	Swarm.bUnfolded = false;
}

// ──────────── 웨이브 시스템 ────────────

void UTPSSwarmSubsystem::StartWaveSystem()
{
	if (bIsActive) { return; }

	CollectSwarmSpawners();

	UE_LOG(LogTemp, Warning, TEXT("[Swarm] StartWaveSystem — Spawners=%d, EnemyType=%s"),
		CachedSpawners.Num(), CachedEnemyType.Get() ? *CachedEnemyType->GetName() : TEXT("None"));

	// 모든 스포너 활성화
	for (const auto& Weak : CachedSpawners)
	{
		if (class ATPSSwarmSpawner* Spawner = Weak.Get())
		{
			Spawner->StartSpawning();
		}
	}

	bIsActive = true;
	CurrentPhase = EWavePhase::Trickle;
	CurrentWaveLevel = 0;
	ElapsedTime = 0.f;
	LastBigWaveTime = 0.f;
}

void UTPSSwarmSubsystem::StopWaveSystem()
{
	bIsActive = false;
	CurrentPhase = EWavePhase::Idle;
}

void UTPSSwarmSubsystem::TickWaveSpawning(float DeltaTime)
{
	if (!bIsActive) { return; }
	if (!CachedSettings.Get()) { return; }

	ElapsedTime += DeltaTime;

	// ── 빅웨이브 주기 도달 → Alert 전환 ──
	const float TimeSinceLastBigWave = ElapsedTime - LastBigWaveTime;
	if (CurrentPhase == EWavePhase::Trickle
		&& TimeSinceLastBigWave >= CachedSettings->BigWavePeriod)
	{
		++CurrentWaveLevel;
		CurrentPhase = EWavePhase::BigWaveAlert;
		LastBigWaveTime = ElapsedTime;

		UE_LOG(LogTemp, Warning, TEXT("[Swarm] BigWaveAlert — Level=%d"), CurrentWaveLevel);
	}

	// ── Alert 시간 경과 → 빅웨이브 → Trickle 복귀 ──
	if (CurrentPhase == EWavePhase::BigWaveAlert)
	{
		const float AlertElapsed = ElapsedTime - LastBigWaveTime;
		if (AlertElapsed >= CachedSettings->BigWaveAlertDuration)
		{
			CurrentPhase = EWavePhase::BigWaveActive;

			const int32 TotalCount = CachedSettings->BigWaveBaseCount
				+ CurrentWaveLevel * CachedSettings->BigWaveCountPerLevel;

			UE_LOG(LogTemp, Warning, TEXT("[Swarm] BigWaveActive — Spawning big wave (%d troops)"), TotalCount);
			SpawnBigWave(TotalCount);

			CurrentPhase = EWavePhase::Trickle;
		}
	}

	// ── 스포너별 주기적 군집 생성 (트리클 + 아군 통합) ──
	for (const auto& Weak : CachedSpawners)
	{
		class ATPSSwarmSpawner* Spawner = Weak.Get();
		if (!Spawner || !Spawner->IsActive()) { continue; }

		if (ElapsedTime - Spawner->LastSpawnTime >= Spawner->GetSpawnInterval())
		{
			SpawnSwarmFromSpawner(Spawner);
			Spawner->LastSpawnTime = ElapsedTime;
		}
	}
}

void UTPSSwarmSubsystem::SpawnSwarmFromSpawner(class ATPSSwarmSpawner* Spawner)
{
	if (!Spawner) { return; }
	AddSwarm(Spawner->SpawnSwarm());
}

void UTPSSwarmSubsystem::SpawnBigWave(int32 TotalCount)
{
	for (const auto& Weak : CachedSpawners)
	{
		class ATPSSwarmSpawner* Spawner = Weak.Get();
		if (!Spawner || Spawner->GetTeam() != ESwarmTeam::Enemy) { continue; }

		AddSwarm(Spawner->SpawnSwarm(TotalCount));
		break;
	}
}

void UTPSSwarmSubsystem::CollectSwarmSpawners()
{
	CachedSpawners.Empty();

	UWorld* pWorld = GetWorld();
	if (!pWorld) { return; }

	for (TActorIterator<ATPSSwarmSpawner> It(pWorld); It; ++It)
	{
		CachedSpawners.Add(*It);
	}
}
