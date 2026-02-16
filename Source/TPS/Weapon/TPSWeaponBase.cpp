#include "TPSWeaponBase.h"
#include "Components/SkeletalMeshComponent.h"

ATPSWeaponBase::ATPSWeaponBase()
{
	if (!WeaponMeshInst)
	{
		WeaponMeshInst = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
		if (ensure(WeaponMeshInst))
		{
			SetRootComponent(WeaponMeshInst);
		}
	}

	AActor::SetActorHiddenInGame(true);
}

void ATPSWeaponBase::Attach(USkeletalMeshComponent* InTargetMesh)
{
	if (!ensure(InTargetMesh)) return;

	AttachToComponent(InTargetMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		EquipSocketName);
	SetActorHiddenInGame(false);
}

void ATPSWeaponBase::Detach(USkeletalMeshComponent* InTargetMesh)
{
	if (!ensure(InTargetMesh)) return;

	AttachToComponent(InTargetMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		UnequipSocketName);
	SetActorHiddenInGame(true);
}

FTransform ATPSWeaponBase::GetMuzzleTransform() const
{
	if (ensure(WeaponMeshInst))
	{
		return WeaponMeshInst->GetSocketTransform(MuzzleSocketName);
	}
	return FTransform::Identity;
}

void ATPSWeaponBase::ConsumeAmmo()
{
	if (CurrentAmmo > 0)
	{
		--CurrentAmmo;
	}
}