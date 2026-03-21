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
	if (ensure(ProjectileMovementInst.Get()))
	{
		// Block 충돌 시 StopSimulating() 대신 Deflect 처리 → PMC 활성 유지
		ProjectileMovementInst->bShouldBounce = true;
		ProjectileMovementInst->Bounciness = 0.f;
		ProjectileMovementInst->OnProjectileBounce.AddDynamic(this, &ATPSPenetratingProjectile::OnBounce);
	}

	// 투사체 간 충돌 무시는 부모에서 TPSCollision::Projectile → Ignore로 이미 처리
}

void ATPSPenetratingProjectile::ActivateProjectile(const FTransform& InMuzzleTransform, const FVector& InDirection, float InDamage, float InSpeed)
{
	CurrentPenetrationCount = 0;
	HitInstanceIndices.Empty();
	HitActors.Empty();
	CachedDirection = InDirection.GetSafeNormal();

	Super::ActivateProjectile(InMuzzleTransform, InDirection, InDamage, InSpeed);

	// Super에서 bShouldBounce=false로 리셋될 수 있으므로 재설정
	if (ensure(ProjectileMovementInst.Get()))
	{
		ProjectileMovementInst->bShouldBounce = true;
	}
}

void ATPSPenetratingProjectile::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	// Block 충돌 후 PMC가 Deflect 처리 → 속도가 반사됨
	// → 원래 발사 방향으로 강제 복원하여 직진 관통
	if (ensure(ProjectileMovementInst.Get()))
	{
		ProjectileMovementInst->Velocity = CachedDirection * ProjectileMovementInst->MaxSpeed;
	}
}

void ATPSPenetratingProjectile::HandleHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult& Hit)
{
	bool bIsEnemy = false;

	// ① HISM(ECS 적) 타격
	if (auto* pHISMComp = Cast<UInstancedStaticMeshComponent>(OtherComp))
	{
		if (Hit.Item != INDEX_NONE)
		{
			bIsEnemy = true;

			// 중복 데미지 방지 — 이미 타격한 인스턴스면 스킵
			if (!HitInstanceIndices.Contains(Hit.Item))
			{
				HitInstanceIndices.Add(Hit.Item);

				UEnemyManagerSubsystem* pEnemyMgr = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>();
				if (ensure(pEnemyMgr))
				{
					FEnemyScheduler* pScheduler = pEnemyMgr->GetScheduler();
					const int32 LODIndex = pScheduler ? pScheduler->FindLODIndexByHISM(pHISMComp) : 0;
					const uint8 LODLevel = static_cast<uint8>(FMath::Max(LODIndex, 0));
					pEnemyMgr->ApplyDamage(Hit.Item, LODLevel, Damage, /*bFromPlayer=*/true);
				}

				++CurrentPenetrationCount;
				UE_LOG(LogTemp, Log, TEXT("[PenetratingProjectile] Hit ECS enemy. Instance=%d Count=%d"), Hit.Item, CurrentPenetrationCount);
			}
		}
	}
	else if (OtherComp->GetCollisionObjectType() == TPSCollision::Enemy
		&& OtherActor != Cast<AActor>(GetInstigator()))
	{
		// ② 액터 기반 적 (Enemy 채널) — Landscape/벽 등은 지형 처리로
		bIsEnemy = true;

		TWeakObjectPtr<AActor> WeakOther(OtherActor);
		if (!HitActors.Contains(WeakOther))
		{
			HitActors.Add(WeakOther);
			UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, nullptr);

			++CurrentPenetrationCount;
			UE_LOG(LogTemp, Log, TEXT("[PenetratingProjectile] Hit actor %s. Count=%d"), *OtherActor->GetName(), CurrentPenetrationCount);
		}
	}

	// ③ 적 관통 — 최대 횟수 체크만 (속도 복원은 OnBounce에서 처리)
	if (bIsEnemy)
	{
		if (MaxPenetrationCount > 0 && CurrentPenetrationCount >= MaxPenetrationCount)
		{
			DeactivateProjectile();
		}
		return;
	}

	// ④ 벽/지형 타격 — 이펙트 + 비활성화
	if (ImpactEffectAsset)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), ImpactEffectAsset,
			Hit.ImpactPoint, Hit.ImpactNormal.Rotation(),
			FVector(1.f), true, true, ENCPoolMethod::AutoRelease);
	}

	DeactivateProjectile();
}
