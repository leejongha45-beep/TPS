#include "Wave/TPSSpawnPointSubsystem.h"
#include "Wave/TPSEnemySpawnPoint.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void UTPSSpawnPointSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UTPSSpawnPointSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	CollectAndSortSpawnPoints(&InWorld);
}

void UTPSSpawnPointSubsystem::Deinitialize()
{
	SortedSpawnPoints.Empty();
	bFinalPhaseActive = false;

	Super::Deinitialize();
}

// ──────────────────────────────────────────────
//  내부 — 수집 + 정렬
// ──────────────────────────────────────────────

void UTPSSpawnPointSubsystem::CollectAndSortSpawnPoints(UWorld* World)
{
	if (!ensure(World)) return;

	// ① 아군 기지 위치 확보 (PlayerStart 활용)
	AActor* pPlayerStart = UGameplayStatics::GetActorOfClass(World, APlayerStart::StaticClass());
	if (ensure(pPlayerStart))
	{
		AllyBaseLocation = pPlayerStart->GetActorLocation();
	}

	// ② TActorIterator로 모든 SpawnPoint 수집
	SortedSpawnPoints.Empty();
	for (TActorIterator<ATPSEnemySpawnPoint> It(World); It; ++It)
	{
		ATPSEnemySpawnPoint* pPoint = *It;
		if (ensure(pPoint))
		{
			SortedSpawnPoints.Add(pPoint);
		}
	}

	// ③ 아군 기지 기준 거리순 정렬 (가까운 → 먼)
	SortedSpawnPoints.Sort([this](ATPSEnemySpawnPoint& A, ATPSEnemySpawnPoint& B)
	{
		const float DistA = FVector::DistSquared(A.GetActorLocation(), AllyBaseLocation);
		const float DistB = FVector::DistSquared(B.GetActorLocation(), AllyBaseLocation);
		return DistA < DistB;
	});

	UE_LOG(LogTemp, Log, TEXT("[SpawnPointSubsystem] Collected %d spawn points, AllyBase at %s"),
	       SortedSpawnPoints.Num(), *AllyBaseLocation.ToString());
}

// ──────────────────────────────────────────────
//  거리 기반 쿼리
// ──────────────────────────────────────────────

TArray<ATPSEnemySpawnPoint*> UTPSSpawnPointSubsystem::GetSpawnPointsByDistance(int32 Count) const
{
	// 정렬된 배열에서 앞부터 활성 포인트 Count개 수집
	// bFinalPhaseActive == false → bIsEnemyHQ 제외 (평소)
	// bFinalPhaseActive == true  → bIsEnemyHQ 포함 (최종 단계)
	TArray<ATPSEnemySpawnPoint*> Result;

	for (ATPSEnemySpawnPoint* Point : SortedSpawnPoints)
	{
		if (!ensure(Point)) continue;
		if (!Point->IsActive()) continue;
		if (!bFinalPhaseActive && Point->IsEnemyHQ()) continue;

		Result.Add(Point);
		if (Result.Num() >= Count) break;
	}

	return Result;
}

TArray<ATPSEnemySpawnPoint*> UTPSSpawnPointSubsystem::GetSpawnPointsByDistanceRange(float MinDist, float MaxDist) const
{
	const float MinDistSq = MinDist * MinDist;
	const float MaxDistSq = MaxDist * MaxDist;

	TArray<ATPSEnemySpawnPoint*> Result;

	for (ATPSEnemySpawnPoint* Point : SortedSpawnPoints)
	{
		if (!ensure(Point)) continue;
		if (!Point->IsActive()) continue;
		if (!bFinalPhaseActive && Point->IsEnemyHQ()) continue;

		const float DistSq = FVector::DistSquared(Point->GetActorLocation(), AllyBaseLocation);
		if (DistSq < MinDistSq) continue;
		if (DistSq > MaxDistSq) break; // 정렬되어 있으므로 이후는 더 멀어짐

		Result.Add(Point);
	}

	return Result;
}

// ──────────────────────────────────────────────
//  특수 쿼리
// ──────────────────────────────────────────────

TArray<ATPSEnemySpawnPoint*> UTPSSpawnPointSubsystem::GetEnemyHQSpawnPoints() const
{
	TArray<ATPSEnemySpawnPoint*> Result;

	for (ATPSEnemySpawnPoint* Point : SortedSpawnPoints)
	{
		if (!ensure(Point)) continue;
		if (!Point->IsActive()) continue;
		if (!Point->IsEnemyHQ()) continue;

		Result.Add(Point);
	}

	return Result;
}

TArray<ATPSEnemySpawnPoint*> UTPSSpawnPointSubsystem::GetRandomSpawnPoints(int32 Count) const
{
	// ① 활성 포인트 전체 수집
	TArray<ATPSEnemySpawnPoint*> ActivePoints;

	for (ATPSEnemySpawnPoint* Point : SortedSpawnPoints)
	{
		if (!ensure(Point)) continue;
		if (!Point->IsActive()) continue;
		if (!bFinalPhaseActive && Point->IsEnemyHQ()) continue;

		ActivePoints.Add(Point);
	}

	// ② Count가 전체 수 이상이면 그대로 반환
	if (Count >= ActivePoints.Num())
	{
		return ActivePoints;
	}

	// ③ Fisher-Yates 셔플 후 앞 Count개 slice
	for (int32 i = ActivePoints.Num() - 1; i > 0; --i)
	{
		const int32 j = FMath::RandRange(0, i);
		ActivePoints.Swap(i, j);
	}

	TArray<ATPSEnemySpawnPoint*> Result;
	Result.Append(ActivePoints.GetData(), Count);

	return Result;
}

// ──────────────────────────────────────────────
//  비활성화 API
// ──────────────────────────────────────────────

void UTPSSpawnPointSubsystem::DeactivateSpawnPoints(const TArray<ATPSEnemySpawnPoint*>& Points)
{
	for (ATPSEnemySpawnPoint* pPoint : Points)
	{
		if (ensure(pPoint))
		{
			pPoint->Deactivate();
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[SpawnPointSubsystem] Deactivated %d spawn points"), Points.Num());
}

// ──────────────────────────────────────────────
//  최종 단계 API
// ──────────────────────────────────────────────

void UTPSSpawnPointSubsystem::ActivateFinalPhase()
{
	bFinalPhaseActive = true;

	UE_LOG(LogTemp, Warning, TEXT("[SpawnPointSubsystem] Final phase activated — HQ spawn points now included in queries"));
}

// ──────────────────────────────────────────────
//  유틸리티
// ──────────────────────────────────────────────

FVector UTPSSpawnPointSubsystem::SnapToGround(const FVector& InLocation, float TraceHeight) const
{
	UWorld* pWorld = GetWorld();
	if (!ensure(pWorld)) return InLocation;

	// ① 위에서 아래로 라인트레이스
	const FVector TraceStart = InLocation + FVector(0.f, 0.f, TraceHeight);
	const FVector TraceEnd = InLocation - FVector(0.f, 0.f, TraceHeight);

	FHitResult HitResult;
	FCollisionQueryParams Params;

	if (pWorld->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params))
	{
		return HitResult.ImpactPoint;
	}

	// ② 트레이스 실패 — 원래 위치 폴백
	return InLocation;
}

void UTPSSpawnPointSubsystem::SetAllyBaseLocation(const FVector& Location)
{
	AllyBaseLocation = Location;

	UE_LOG(LogTemp, Log, TEXT("[SpawnPointSubsystem] AllyBaseLocation set to %s"), *AllyBaseLocation.ToString());
}
