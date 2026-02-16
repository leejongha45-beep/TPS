#include "TPSFireComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Core/Subsystem/TPSProjectilePoolSubsystem.h"
#include "Weapon/TPSWeaponBase.h"
#include "Weapon/Projectile/TPSProjectileBase.h"

DECLARE_LOG_CATEGORY_EXTERN(FireLog, Log, All);

DEFINE_LOG_CATEGORY(FireLog);

void UTPSFireComponent::StartFire(ATPSWeaponBase* InWeapon, TFunction<void (FVector&, FRotator&)> InViewPointGetter)
{
	if (bIsFiring) return;
	if (!ensure(InWeapon)) return;

	if (!WeaponRef.Get())
	{
		WeaponRef = InWeapon;
		ATPSWeaponBase* pWeapon = WeaponRef.Get();
		if (ensure(pWeapon))
		{
			if (!pWeapon->HasAmmo()) return;
		}
	}

	ViewPointGetter = InViewPointGetter;

	bIsFiring = true;

	OnFireStateChangedDelegate.Broadcast(true);

	// 일단 한발쏘고
	FireOnce();

	// 연사
	GetWorld()->GetTimerManager().SetTimer(
		FireTimerHandle, this, &UTPSFireComponent::FireOnce,
		InWeapon->GetFireInterval(), true);

	UE_LOG(FireLog, Log, TEXT("[StartFire] Firing started. FireInterval: %.3f"), InWeapon->GetFireInterval());
}

void UTPSFireComponent::StopFire()
{
	if (!bIsFiring) return;

	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	bIsFiring = false;

	WeaponRef.Reset();
	OnFireStateChangedDelegate.Broadcast(false);

	UE_LOG(FireLog, Log, TEXT("[StopFire] Firing stopped."));
}

void UTPSFireComponent::FireOnce()
{
	ATPSWeaponBase* pWeapon = WeaponRef.Get();
	if (!ensure(pWeapon))
	{
		StopFire();
		return;
	}

	if (!pWeapon->HasAmmo())
	{
		UE_LOG(FireLog, Log, TEXT("[FireOnce] Out of ammo. Stopping fire."));
		StopFire();
		return;
	}

	pWeapon->ConsumeAmmo();

	const FTransform MuzzleTransform = pWeapon->GetMuzzleTransform();
	const FVector ShotDirection = CalculateShotDirection(MuzzleTransform.GetLocation());

	SpawnMuzzleEffect(MuzzleTransform);
	ActivateProjectileFromPool(MuzzleTransform, ShotDirection, pWeapon);
}

FVector UTPSFireComponent::CalculateShotDirection(const FVector& InMuzzleLocation) const
{
	FVector ViewLocation;
	FRotator ViewRotation;
	
	ViewPointGetter(ViewLocation, ViewRotation);

	const FVector TraceStart = ViewLocation;
	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * 10000.f;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.AddIgnoredActor(WeaponRef.Get());

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	const FVector TargetPoint = bHit ? HitResult.ImpactPoint : TraceEnd;
	return (TargetPoint - InMuzzleLocation).GetSafeNormal();
}

void UTPSFireComponent::SpawnMuzzleEffect(const FTransform& InMuzzleTransform)
{
	if (!MuzzleFlashEffectAsset) return;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(), MuzzleFlashEffectAsset,
		InMuzzleTransform.GetLocation(), InMuzzleTransform.GetRotation().Rotator(),
		FVector(1.f), true, true, ENCPoolMethod::AutoRelease);
}

void UTPSFireComponent::ActivateProjectileFromPool(const FTransform& InMuzzleTransform, const FVector& InDirection, ATPSWeaponBase* InWeapon)
{
	UTPSProjectilePoolSubsystem* pPool = GetWorld()->GetSubsystem<UTPSProjectilePoolSubsystem>();
	if (!ensure(pPool)) return;

	ATPSProjectileBase* pProjectile = pPool->GetProjectile();
	if (!ensure(pProjectile)) return;

	pProjectile->SetInstigator(Cast<APawn>(GetOwner()));
	pProjectile->SetOwner(GetOwner());
	pProjectile->ActivateProjectile(InMuzzleTransform, InDirection, InWeapon->GetDamage(), InWeapon->GetProjectileSpeed());
}