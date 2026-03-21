#include "Weapon/Projectile/TPSPenetratingProjectile.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "ECS/Scheduler/EnemyManagerSubsystem.h"
#include "ECS/Scheduler/EnemyScheduler.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Utils/Collision/TPSCollisionChannels.h"

ATPSPenetratingProjectile::ATPSPenetratingProjectile()
{
	if (ensure(CollisionComponentInst.Get()))
	{
		// Enemy 채널 → Overlap: 적을 물리적으로 뚫고 지나감
		CollisionComponentInst->SetCollisionResponseToChannel(TPSCollision::Enemy, ECR_Overlap);
		CollisionComponentInst->SetGenerateOverlapEvents(true);

		CollisionComponentInst->OnComponentBeginOverlap.AddDynamic(
			this, &ATPSPenetratingProjectile::OnOverlapEnemy);
	}
}

void ATPSPenetratingProjectile::ActivateProjectile(const FTransform& InMuzzleTransform, const FVector& InDirection, float InDamage, float InSpeed)
{
	CurrentPenetrationCount = 0;
	HitInstanceIndices.Empty();
	HitActors.Empty();

	if (PenetratingTrailAsset)
	{
		TrailEffectAsset = PenetratingTrailAsset;
	}

	Super::ActivateProjectile(InMuzzleTransform, InDirection, InDamage, InSpeed);
}

void ATPSPenetratingProjectile::OnOverlapEnemy(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == Cast<AActor>(GetInstigator()) || OtherActor == GetOwner())
	{
		return;
	}

	// ① HISM(ECS 적) 타격
	if (auto* pHISMComp = Cast<UInstancedStaticMeshComponent>(OtherComp))
	{
		const int32 Item = SweepResult.Item;
		if (Item != INDEX_NONE && !HitInstanceIndices.Contains(Item))
		{
			HitInstanceIndices.Add(Item);

			UEnemyManagerSubsystem* pEnemyMgr = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>();
			if (ensure(pEnemyMgr))
			{
				FEnemyScheduler* pScheduler = pEnemyMgr->GetScheduler();
				const int32 LODIndex = pScheduler ? pScheduler->FindLODIndexByHISM(pHISMComp) : 0;
				const uint8 LODLevel = static_cast<uint8>(FMath::Max(LODIndex, 0));
				pEnemyMgr->ApplyDamage(Item, LODLevel, Damage, /*bFromPlayer=*/true);
			}

			++CurrentPenetrationCount;
			UE_LOG(LogTemp, Log, TEXT("[PenetratingProjectile] Overlap ECS enemy. Instance=%d Count=%d"), Item, CurrentPenetrationCount);
		}
	}
	// ② 액터 기반 적 (Enemy 채널)
	else if (OtherComp && OtherComp->GetCollisionObjectType() == TPSCollision::Enemy)
	{
		TWeakObjectPtr<AActor> WeakOther(OtherActor);
		if (!HitActors.Contains(WeakOther))
		{
			HitActors.Add(WeakOther);
			UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, nullptr);

			++CurrentPenetrationCount;
			UE_LOG(LogTemp, Log, TEXT("[PenetratingProjectile] Overlap actor %s. Count=%d"), *OtherActor->GetName(), CurrentPenetrationCount);
		}
	}

	// ③ 최대 관통 횟수 체크
	if (MaxPenetrationCount > 0 && CurrentPenetrationCount >= MaxPenetrationCount)
	{
		DeactivateProjectile();
	}
}

void ATPSPenetratingProjectile::HandleHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult& Hit)
{
	// Enemy는 Overlap으로 처리되므로 여기에 오는 건 벽/지형만
	// → 이펙트 + 비활성화
	if (ImpactEffectAsset)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), ImpactEffectAsset,
			Hit.ImpactPoint, Hit.ImpactNormal.Rotation(),
			FVector(1.f), true, true, ENCPoolMethod::AutoRelease);
	}

	DeactivateProjectile();
}
