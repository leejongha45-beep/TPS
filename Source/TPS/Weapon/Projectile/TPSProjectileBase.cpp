#include "TPSProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Core/Subsystem/TPSProjectilePoolSubsystem.h"

DECLARE_LOG_CATEGORY_EXTERN(ProjectileLog, Log, All);
DEFINE_LOG_CATEGORY(ProjectileLog);

ATPSProjectileBase::ATPSProjectileBase()
{
	if (!CollisionComponentInst)
	{
		CollisionComponentInst = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
		if (ensure(CollisionComponentInst))
		{
			CollisionComponentInst->InitSphereRadius(5.f);
			CollisionComponentInst->SetCollisionProfileName(TEXT("BlockAllDynamic"));
			SetRootComponent(CollisionComponentInst);

			CollisionComponentInst->OnComponentHit.AddDynamic(this, &ATPSProjectileBase::OnHit);
		}
	}

	if (!ProjectileMovementInst)
	{
		ProjectileMovementInst = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
		if (ensure(ProjectileMovementInst))
		{
			ProjectileMovementInst->UpdatedComponent = CollisionComponentInst;
			ProjectileMovementInst->bRotationFollowsVelocity = true;
			ProjectileMovementInst->bShouldBounce = false;
			ProjectileMovementInst->ProjectileGravityScale = 0.f;
			ProjectileMovementInst->InitialSpeed = 0.f;
			ProjectileMovementInst->MaxSpeed = 0.f;
			ProjectileMovementInst->bAutoActivate = false;
		}
	}

	AActor::SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	AActor::SetActorTickEnabled(false);
}

void ATPSProjectileBase::ActivateProjectile(const FTransform& InMuzzleTransform, const FVector& InDirection, float InDamage, float InSpeed)
{
	Damage = InDamage;

	if (ensure(CollisionComponentInst))
	{
		CollisionComponentInst->MoveIgnoreActors.Empty();
		if (GetInstigator())
		{
			CollisionComponentInst->MoveIgnoreActors.Add(GetInstigator());
		}
	}

	SetActorLocationAndRotation(InMuzzleTransform.GetLocation(), InDirection.Rotation());
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	if (ensure(ProjectileMovementInst))
	{
		ProjectileMovementInst->InitialSpeed = InSpeed;
		ProjectileMovementInst->MaxSpeed = InSpeed;
		ProjectileMovementInst->Velocity = InDirection * InSpeed;
		ProjectileMovementInst->Activate(true);
	}

	GetWorldTimerManager().SetTimer(
		LifeSpanTimerHandle, this, &ATPSProjectileBase::OnLifeSpanExpired, LifeSpan, false);
}

void ATPSProjectileBase::DeactivateProjectile()
{
	GetWorldTimerManager().ClearTimer(LifeSpanTimerHandle);

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	if (ensure(ProjectileMovementInst))
	{
		ProjectileMovementInst->StopMovementImmediately();
		ProjectileMovementInst->Deactivate();
	}

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
	if (!OtherActor || OtherActor == GetInstigator() || OtherActor == GetOwner())
	{
		return;
	}

	UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, nullptr);

	if (ImpactEffectAsset)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), ImpactEffectAsset,
			Hit.ImpactPoint, Hit.ImpactNormal.Rotation(),
			FVector(1.f), true, true, ENCPoolMethod::AutoRelease);
	}

	DeactivateProjectile();
}

void ATPSProjectileBase::OnLifeSpanExpired()
{
	DeactivateProjectile();
}