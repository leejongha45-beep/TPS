#include "TPSFireComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Component/Data/TPSPlayerStateComponent.h"
#include "Component/Action/TPSEquipComponent.h"
#include "Weapon/TPSWeaponBase.h"
#include "Weapon/Projectile/TPSProjectileBase.h"
#include "Core/Subsystem/TPSProjectilePoolSubsystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

DECLARE_LOG_CATEGORY_EXTERN(FireLog, Log, All);

DEFINE_LOG_CATEGORY(FireLog);

void UTPSFireComponent::StartFire()
{
	if (bIsFiring) return;

	ACharacter* pOwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!ensure(pOwnerCharacter)) return;

	UTPSPlayerStateComponent* pStateComp = pOwnerCharacter->FindComponentByClass<UTPSPlayerStateComponent>();
	if (!ensure(pStateComp)) return;

	if (!pStateComp->HasState(EActionState::Equipping)) return;

	UTPSEquipComponent* pEquipComp = pOwnerCharacter->FindComponentByClass<UTPSEquipComponent>();
	if (!ensure(pEquipComp)) return;

	ATPSWeaponBase* pWeapon = pEquipComp->GetWeaponActor();
	if (!ensure(pWeapon)) return;

	if (!pWeapon->HasAmmo()) return;

	bIsFiring = true;
	pStateComp->AddState(EActionState::Firing);

	FireOnce();

	GetWorld()->GetTimerManager().SetTimer(
		FireTimerHandle, this, &UTPSFireComponent::FireOnce,
		pWeapon->GetFireInterval(), true);

	UE_LOG(FireLog, Log, TEXT("[StartFire] Firing started. FireInterval: %.3f"), pWeapon->GetFireInterval());
}

void UTPSFireComponent::StopFire()
{
	if (!bIsFiring) return;

	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	bIsFiring = false;

	ACharacter* pOwnerCharacter = Cast<ACharacter>(GetOwner());
	if (ensure(pOwnerCharacter))
	{
		UTPSPlayerStateComponent* pStateComp = pOwnerCharacter->FindComponentByClass<UTPSPlayerStateComponent>();
		if (ensure(pStateComp))
		{
			pStateComp->RemoveState(EActionState::Firing);
		}
	}

	UE_LOG(FireLog, Log, TEXT("[StopFire] Firing stopped."));
}

void UTPSFireComponent::FireOnce()
{
	ACharacter* pOwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!ensure(pOwnerCharacter))
	{
		StopFire();
		return;
	}

	UTPSEquipComponent* pEquipComp = pOwnerCharacter->FindComponentByClass<UTPSEquipComponent>();
	if (!ensure(pEquipComp))
	{
		StopFire();
		return;
	}

	ATPSWeaponBase* pWeapon = pEquipComp->GetWeaponActor();
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
	ActivateProjectileFromPool(MuzzleTransform, ShotDirection);
}

FVector UTPSFireComponent::CalculateShotDirection(const FVector& InMuzzleLocation) const
{
	const APawn* pOwnerPawn = Cast<APawn>(GetOwner());
	if (!ensure(pOwnerPawn)) return FVector::ForwardVector;

	APlayerController* pController = Cast<APlayerController>(pOwnerPawn->GetController());
	if (!ensure(pController)) return FVector::ForwardVector;

	FVector CameraLocation;
	FRotator CameraRotation;
	pController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FVector TraceStart = CameraLocation;
	const FVector TraceEnd = CameraLocation + CameraRotation.Vector() * 10000.f;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

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

void UTPSFireComponent::ActivateProjectileFromPool(const FTransform& InMuzzleTransform, const FVector& InDirection)
{
	UTPSProjectilePoolSubsystem* pPool = GetWorld()->GetSubsystem<UTPSProjectilePoolSubsystem>();
	if (!ensure(pPool)) return;

	ATPSProjectileBase* pProjectile = pPool->GetProjectile();
	if (!ensure(pProjectile)) return;

	ACharacter* pOwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!ensure(pOwnerCharacter)) return;

	UTPSEquipComponent* pEquipComp = pOwnerCharacter->FindComponentByClass<UTPSEquipComponent>();
	if (!ensure(pEquipComp)) return;

	ATPSWeaponBase* pWeapon = pEquipComp->GetWeaponActor();
	if (!ensure(pWeapon)) return;

	pProjectile->SetInstigator(Cast<APawn>(GetOwner()));
	pProjectile->SetOwner(GetOwner());
	pProjectile->ActivateProjectile(InMuzzleTransform, InDirection, pWeapon->GetDamage(), pWeapon->GetProjectileSpeed());
}