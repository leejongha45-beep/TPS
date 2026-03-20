#pragma once

#include "CoreMinimal.h"
#include "Character/Base/TPSSoldierBase.h"
#include "TPSSoldierNPC.generated.h"

UCLASS(Blueprintable)
class TPS_API ATPSSoldierNPC : public ATPSSoldierBase
{
	GENERATED_BODY()

public:
	ATPSSoldierNPC(const FObjectInitializer& ObjectInitializer);

	virtual void StartFire() override;
	virtual void Reload() override;

	/** 소속 군집 ID — Unfold 시 설정, Fold 시 매칭용 */
	int32 OwnerSwarmID = INDEX_NONE;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 스폰할 무기 클래스 (BP에서 설정) */
	UPROPERTY(EditDefaultsOnly, Category = "NPC|Weapon")
	TSubclassOf<class ATPSWeaponBase> WeaponClass;

	/** 스폰된 무기 인스턴스 */
	UPROPERTY()
	TObjectPtr<class ATPSWeaponBase> SpawnedWeapon;
};
