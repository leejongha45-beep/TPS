#include "TPSProjectileBase.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/Projectile/TPSProjectilePoolSubsystem.h"
#include "ECS/Scheduler/EnemyManagerSubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Core/Subsystem/TPSTargetSubsystem.h"
#include "Utils/Collision/TPSCollisionChannels.h"

DECLARE_LOG_CATEGORY_EXTERN(ProjectileLog, Log, All);
DEFINE_LOG_CATEGORY(ProjectileLog);

ATPSProjectileBase::ATPSProjectileBase()
{
	// ① 충돌 컴포넌트 (SphereCollision + OnHit 바인딩)
	if (!CollisionComponentInst)
	{
		CollisionComponentInst = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
		if (ensure(CollisionComponentInst.Get()))
		{
			CollisionComponentInst->InitSphereRadius(5.f);
			CollisionComponentInst->SetCollisionObjectType(TPSCollision::Projectile);
			CollisionComponentInst->SetCollisionResponseToAllChannels(ECR_Block);
			CollisionComponentInst->SetCollisionResponseToChannel(TPSCollision::Projectile, ECR_Ignore);
			SetRootComponent(CollisionComponentInst.Get());

			CollisionComponentInst->OnComponentHit.AddDynamic(this, &ATPSProjectileBase::OnHit);
		}
	}

	// ② ProjectileMovement 컴포넌트 (비활성 상태로 생성)
	if (!ProjectileMovementInst)
	{
		ProjectileMovementInst = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
		if (ensure(ProjectileMovementInst.Get()))
		{
			ProjectileMovementInst->UpdatedComponent = CollisionComponentInst.Get();
			ProjectileMovementInst->bRotationFollowsVelocity = true;
			ProjectileMovementInst->bShouldBounce = false;
			ProjectileMovementInst->ProjectileGravityScale = 0.f;
			ProjectileMovementInst->InitialSpeed = 0.f;
			ProjectileMovementInst->MaxSpeed = 0.f;
			ProjectileMovementInst->bAutoActivate = false;
		}
	}

	// ③ 초기 상태: 숨김 + 충돌 비활성 + Tick 비활성 (풀 대기)
	AActor::SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	AActor::SetActorTickEnabled(false);
}

void ATPSProjectileBase::ActivateProjectile(const FTransform& InMuzzleTransform, const FVector& InDirection, float InDamage, float InSpeed)
{
	// ① 데미지 설정 + 충돌 무시 목록 갱신
	Damage = InDamage;

	if (ensure(CollisionComponentInst.Get()))
	{
		CollisionComponentInst->MoveIgnoreActors.Empty();
		if (GetInstigator())
		{
			CollisionComponentInst->MoveIgnoreActors.Add(Cast<AActor>(GetInstigator()));
		}

		// NPC 무시 — 프로젝타일이 NPC를 뚫고 지나감
		if (UTPSTargetSubsystem* TargetSS = GetWorld()->GetSubsystem<UTPSTargetSubsystem>())
		{
			for (const auto& NPC : TargetSS->GetNPCs())
			{
				if (NPC.Get())
				{
					CollisionComponentInst->MoveIgnoreActors.Add(NPC.Get());
				}
			}
		}
	}

	// ② 위치/회전 + 가시성/충돌/Tick 활성화
	SetActorLocationAndRotation(InMuzzleTransform.GetLocation(), InDirection.Rotation());
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	// ③ ProjectileMovement 속도 설정 + 활성화
	if (ensure(ProjectileMovementInst.Get()))
	{
		ProjectileMovementInst->InitialSpeed = InSpeed;
		ProjectileMovementInst->MaxSpeed = InSpeed;
		ProjectileMovementInst->Velocity = InDirection * InSpeed;
		ProjectileMovementInst->Activate(true);
	}

	// ④ 수명 타이머 등록
	GetWorldTimerManager().SetTimer(
		LifeSpanTimerHandle, this, &ATPSProjectileBase::OnLifeSpanExpired, LifeSpan, false);
}

void ATPSProjectileBase::DeactivateProjectile()
{
	// ① 타이머 해제 + 숨김/충돌/Tick 비활성화
	GetWorldTimerManager().ClearTimer(LifeSpanTimerHandle);

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	// ② 이동 즉시 정지 + 컴포넌트 비활성화
	if (ensure(ProjectileMovementInst.Get()))
	{
		ProjectileMovementInst->StopMovementImmediately();
		ProjectileMovementInst->Deactivate();
	}

	// ③ 맵 외부로 이동 + 풀에 반환
	SetActorLocation(FVector(0.f, 0.f, -10000.f));

	UTPSProjectilePoolSubsystem* pPoolSubsystem = GetWorld()->GetSubsystem<UTPSProjectilePoolSubsystem>();
	if (ensure(pPoolSubsystem))
	{
		pPoolSubsystem->ReturnProjectile(this);
	}
}

void ATPSProjectileBase::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == Cast<AActor>(GetInstigator()) || OtherActor == GetOwner())
	{
		return;
	}

	HandleHit(OtherActor, OtherComp, Hit);
}

void ATPSProjectileBase::HandleHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult& Hit)
{
	// ① 대상에 데미지 적용
	if (auto* pHISMComp = Cast<UInstancedStaticMeshComponent>(OtherComp))
	{
		// HISM 충돌 → ECS 데미지 경로 (LOD 인덱스 역조회)
		if (Hit.Item != INDEX_NONE)
		{
			UEnemyManagerSubsystem* pEnemyMgr = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>();
			if (ensure(pEnemyMgr))
			{
				FEnemyScheduler* pScheduler = pEnemyMgr->GetScheduler();
				const int32 LODIndex = pScheduler ? pScheduler->FindLODIndexByHISM(pHISMComp) : 0;
				const uint8 LODLevel = static_cast<uint8>(FMath::Max(LODIndex, 0));
				pEnemyMgr->ApplyDamage(Hit.Item, LODLevel, Damage, /*bFromPlayer=*/true);
			}
		}
	}
	else if (OtherComp->GetCollisionObjectType() == TPSCollision::Enemy)
	{
		// Enemy 채널 액터 기반 데미지
		UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, nullptr);
	}

	// ② 충돌 지점에 이펙트 스폰
	if (ImpactEffectAsset)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), ImpactEffectAsset,
			Hit.ImpactPoint, Hit.ImpactNormal.Rotation(),
			FVector(1.f), true, true, ENCPoolMethod::AutoRelease);
	}

	// ③ 투사체 비활성화 (풀 반환)
	DeactivateProjectile();
}

void ATPSProjectileBase::OnLifeSpanExpired()
{
	DeactivateProjectile();
}