#include "TPSRifle.h"
#include "Components/SkeletalMeshComponent.h"

ATPSRifle::ATPSRifle()
{
	if (!WeaponMeshInst)
	{
		WeaponMeshInst = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
		if (ensure(WeaponMeshInst))
		{
			SetRootComponent(WeaponMeshInst);
		}
	}
}

void ATPSRifle::Attach(USkeletalMeshComponent* InTargetMesh)
{
	if (!ensure(InTargetMesh)) return;

	AttachToComponent(InTargetMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		EquipSocketName);
	SetActorHiddenInGame(false);
}

void ATPSRifle::Detach(USkeletalMeshComponent* InTargetMesh)
{
	if (!ensure(InTargetMesh)) return;

	AttachToComponent(InTargetMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		UnequipSocketName);
	SetActorHiddenInGame(true);
}