#include "TPSWeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "UI/ViewModel/AmmoViewModel.h"

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

void ATPSWeaponBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// ① AmmoViewModel 생성 + 초기 탄약값 세팅
	if (!AmmoViewModelInst)
	{
		AmmoViewModelInst = NewObject<UAmmoViewModel>(this);
		if (ensure(AmmoViewModelInst))
		{
			AmmoViewModelInst->SetAmmo(CurrentAmmo, MaxAmmo);
		}
	}
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

	if (ensure(AmmoViewModelInst))
	{
		AmmoViewModelInst->SetAmmo(CurrentAmmo, MaxAmmo);
	}
}

void ATPSWeaponBase::ReloadAmmo()
{
	CurrentAmmo = MaxAmmo;

	if (ensure(AmmoViewModelInst))
	{
		AmmoViewModelInst->SetAmmo(CurrentAmmo, MaxAmmo);
	}
}