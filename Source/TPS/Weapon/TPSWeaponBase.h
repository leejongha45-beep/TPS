#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/Interface/Action/Attachable.h"
#include "TPSWeaponBase.generated.h"

UCLASS()
class TPS_API ATPSWeaponBase : public AActor, public IAttachable
{
	GENERATED_BODY()

public:
	ATPSWeaponBase();

	virtual void Attach(USkeletalMeshComponent* InTargetMesh) override;
	virtual void Detach(USkeletalMeshComponent* InTargetMesh) override;

	FTransform GetMuzzleTransform() const;
	void ConsumeAmmo();

	FORCEINLINE float GetFireInterval() const { return FireInterval; }
	FORCEINLINE float GetDamage() const { return WeaponDamage; }
	FORCEINLINE float GetProjectileSpeed() const { return ProjectileSpeed; }
	FORCEINLINE int32 GetCurrentAmmo() const { return CurrentAmmo; }
	FORCEINLINE int32 GetMaxAmmo() const { return MaxAmmo; }
	FORCEINLINE bool HasAmmo() const { return CurrentAmmo > 0; }

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Mesh")
	TObjectPtr<class USkeletalMeshComponent> WeaponMeshInst;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Socket")
	FName EquipSocketName = TEXT("weapon_r_Handle");

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Socket")
	FName UnequipSocketName = TEXT("spine_01");

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Socket")
	FName MuzzleSocketName = TEXT("Muzzle");

	// 충돌 범위 (공격 범위)
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Fire")
	float FireInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Fire")
	float WeaponDamage = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Fire")
	float ProjectileSpeed = 10000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Ammo")
	int32 MaxAmmo = 60;

	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon|Ammo")
	int32 CurrentAmmo = 60;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Ammo")
	float ReloadTime = 2.5f;
};