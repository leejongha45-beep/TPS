#include "UI/ViewModel/MinimapViewModel.h"
#include "Core/Subsystem/TPSSwarmSubsystem.h"
#include "Core/Subsystem/TPSTargetSubsystem.h"
#include "Actor/AllyBase/TPSAllyBase.h"
#include "Actor/EnemyBase/TPSEnemyBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

void UMinimapViewModel::SetMapBounds(const FVector2D& InCenter, float InExtent)
{
	MapWorldCenter = InCenter;
	MapWorldExtent = InExtent;
}

void UMinimapViewModel::Update(UWorld* InWorld, const FVector2D& InMapCenter, float InMapExtent)
{
	if (!InWorld) return;

	MapWorldCenter = InMapCenter;
	MapWorldExtent = InMapExtent;

	// ① 기지 마커 (1회 초기화)
	if (!bBasesInitialized)
	{
		UTPSTargetSubsystem* pTargetSS = InWorld->GetSubsystem<UTPSTargetSubsystem>();
		if (pTargetSS)
		{
			BaseMarkers.Reset();

			for (const auto& Base : pTargetSS->GetAllyBases())
			{
				if (Base.Get())
				{
					FMinimapMarkerData Marker;
					Marker.Position = WorldToNormalized(Base->GetActorLocation());
					Marker.Color = FLinearColor(0.2f, 0.4f, 1.f, 1.f);
					Marker.Size = 20.f;
					BaseMarkers.Add(Marker);
				}
			}

			for (const auto& Base : pTargetSS->GetEnemyBases())
			{
				if (Base.Get())
				{
					FMinimapMarkerData Marker;
					Marker.Position = WorldToNormalized(Base->GetActorLocation());
					Marker.Color = FLinearColor(1.f, 0.2f, 0.2f, 1.f);
					Marker.Size = 20.f;
					BaseMarkers.Add(Marker);
				}
			}

			if (BaseMarkers.Num() > 0)
			{
				bBasesInitialized = true;
			}
		}
	}

	// ② 군집 마커
	UTPSSwarmSubsystem* pSwarmSS = InWorld->GetSubsystem<UTPSSwarmSubsystem>();
	if (pSwarmSS)
	{
		const TArray<FSwarm>& Swarms = pSwarmSS->GetSwarms();

		SwarmMarkers.SetNum(Swarms.Num());

		for (int32 i = 0; i < Swarms.Num(); ++i)
		{
			const FSwarm& Swarm = Swarms[i];
			FMinimapMarkerData& Marker = SwarmMarkers[i];

			if (Swarm.TroopCount <= 0 || Swarm.bUnfolded)
			{
				Marker.bVisible = false;
				continue;
			}

			Marker.bVisible = true;
			Marker.Position = WorldToNormalized(Swarm.Position);
			Marker.Color = (Swarm.Team == ESwarmTeam::Enemy)
				? FLinearColor(1.f, 0.3f, 0.3f, 0.9f)
				: FLinearColor(0.3f, 0.5f, 1.f, 0.9f);
			Marker.Size = FMath::Clamp(6.f + Swarm.TroopCount * 0.03f, 6.f, 20.f);
		}
	}

	// ③ 플레이어 위치
	bPlayerValid = false;
	APlayerController* pPC = InWorld->GetFirstPlayerController();
	if (pPC)
	{
		APawn* pPawn = pPC->GetPawn();
		if (pPawn)
		{
			PlayerPosition = WorldToNormalized(pPawn->GetActorLocation());
			bPlayerValid = true;
		}
	}
}

FVector2D UMinimapViewModel::WorldToNormalized(const FVector& WorldPos) const
{
	const float NormX = (WorldPos.X - MapWorldCenter.X) / MapWorldExtent;
	const float NormY = (WorldPos.Y - MapWorldCenter.Y) / MapWorldExtent;

	return FVector2D(NormX * 0.5f + 0.5f, NormY * 0.5f + 0.5f);
}
