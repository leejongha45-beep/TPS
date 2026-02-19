#pragma once

#include "CoreMinimal.h"
#include "Pawn/Character/Base/TPSCharacterBase.h"
#include "Utils/Interface/Action/Aimable.h"
#include "Utils/Interface/Action/Equippable.h"
#include "Utils/Interface/Action/Fireable.h"
#include "TPSSoldierBase.generated.h"

/**
 * 전투 가능 캐릭터 베이스
 * - 조준/장착/사격 인터페이스 구현
 * - 전투 컴포넌트: Equip, Fire, AnimLayer
 * - 자식: ATPSPlayer (플레이어), 향후 ATPSSoldierNPC (적 NPC)
 */
UCLASS(Abstract)
class TPS_API ATPSSoldierBase
	: public ATPSCharacterBase, public IAimable, public IEquippable, public IFireable
{
	GENERATED_BODY()

public:
	ATPSSoldierBase(const FObjectInitializer& ObjectInitializer);

	FORCEINLINE class UTPSEquipComponent* GetEquipComponent() const { return EquipComponentInst; }
	FORCEINLINE class UTPSFireComponent* GetFireComponent() const { return FireComponentInst; }
	FORCEINLINE class UTPSAnimLayerComponent* GetAnimLayerComponent() const { return AnimLayerComponentInst; }

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void CreateDefaultComponents() override;

	/** 델리게이트 바인딩 — 자식에서 추가 바인딩 시 Super:: 호출 */
	virtual void BindDelegate();

#pragma region Component
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Action")
	TObjectPtr<class UTPSEquipComponent> EquipComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Action")
	TObjectPtr<class UTPSFireComponent> FireComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Animation")
	TObjectPtr<class UTPSAnimLayerComponent> AnimLayerComponentInst;
#pragma endregion

#pragma region IAimable
	virtual void StartAim() override;
	virtual void StopAim() override;
#pragma endregion

#pragma region IEquippable
	virtual void Equip() override;
	virtual void Unequip() override;
#pragma endregion

#pragma region IFireable
	virtual void StartFire() override;
	virtual void StopFire() override;
#pragma endregion

#pragma region EquipCallback
	virtual void OnEquipStateChanged(bool bIsEquipped);
#pragma endregion

#pragma region FireCallback
	virtual void OnFireStateChanged(bool bIsFiring);
#pragma endregion
};
