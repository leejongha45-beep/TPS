#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/Interface/Action/Attachable.h"
#include "TPSWeaponBase.generated.h"

/**
 * 무기 베이스 클래스
 * - IAttachable 구현 — 캐릭터 메시 소켓에 부착/분리
 * - 사격 스탯 (FireInterval, Damage, ProjectileSpeed)
 * - 탄약 관리 (CurrentAmmo, MaxAmmo, ReloadTime)
 * - 서브클래스: ATPSRifle, (향후 Shotgun, Pistol 등)
 */
UCLASS()
class TPS_API ATPSWeaponBase : public AActor, public IAttachable
{
	GENERATED_BODY()

public:
	ATPSWeaponBase();

	/** 장착 소켓에 부착 */
	virtual void Attach(USkeletalMeshComponent* InTargetMesh) override;

	/** 해제 소켓에 분리 */
	virtual void Detach(USkeletalMeshComponent* InTargetMesh) override;

	/** 머즐 소켓의 월드 트랜스폼 반환 */
	FTransform GetMuzzleTransform() const;

	/** 탄약 1발 소모 */
	void ConsumeAmmo();

	FORCEINLINE float GetFireInterval() const { return FireInterval; }
	FORCEINLINE float GetDamage() const { return WeaponDamage; }
	FORCEINLINE float GetProjectileSpeed() const { return ProjectileSpeed; }
	FORCEINLINE int32 GetCurrentAmmo() const { return CurrentAmmo; }
	FORCEINLINE int32 GetMaxAmmo() const { return MaxAmmo; }
	FORCEINLINE bool HasAmmo() const { return CurrentAmmo > 0; }

protected:
	/** 무기 메시 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Mesh")
	TObjectPtr<class USkeletalMeshComponent> WeaponMeshInst;

	/** 장착 시 부착할 소켓 이름 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Socket")
	FName EquipSocketName = TEXT("weapon_r_Handle");

	/** 해제 시 부착할 소켓 이름 (등/홀스터) */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Socket")
	FName UnequipSocketName = TEXT("spine_01");

	/** 머즐 소켓 이름 (발사체 스폰 위치) */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Socket")
	FName MuzzleSocketName = TEXT("Muzzle");

	/** 발사 간격 (초) — 연사 속도 결정 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Fire")
	float FireInterval = 0.1f;

	/** 1발당 데미지 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Fire")
	float WeaponDamage = 20.f;

	/** 발사체 속도 (cm/s) */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Fire")
	float ProjectileSpeed = 10000.f;

	/** 최대 탄약 수 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Ammo")
	int32 MaxAmmo = 60;

	/** 현재 탄약 수 */
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon|Ammo")
	int32 CurrentAmmo = 60;

	/** 재장전 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Ammo")
	float ReloadTime = 2.5f;
};