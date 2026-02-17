#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utils/Interface/Action/Attachable.h"
#include "Weapon/TPSWeaponBase.h"
#include "TPSEquipComponent.generated.h"

/** 장착 몽타주 재생 시 브로드캐스트 (bEquip: true=장착, false=해제) */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquipMontagePlay, bool /* bEquip */);

/** 장착 상태 변경 완료 시 브로드캐스트 (bIsEquipped: 최종 장착 상태) */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquipStateChanged, bool /* bIsEquipped */);

/**
 * 무기 장착/해제 관리 컴포넌트
 * - RequestToggle → 몽타주 재생 요청 → OnMontageFinished에서 최종 전환
 * - IAttachable 인터페이스로 무기 Attach/Detach
 * - AnimLayer 전환은 델리게이트를 통해 AnimLayerComponent가 처리
 *
 * 주의: OnMontageFinished 타이밍이 AnimInstance의 bIsEquipping에 의존
 * → Known Tech Debt (MEMORY.md 참조)
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TPS_API UTPSEquipComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FORCEINLINE bool GetIsEquipTransitioning() const { return bIsEquipTransitioning; }
	FORCEINLINE void SetWeaponInterface(TScriptInterface<IAttachable> InWeapon) { WeaponInterface = InWeapon; }
	FORCEINLINE ATPSWeaponBase* GetWeaponActor() const { return Cast<ATPSWeaponBase>(WeaponInterface.GetObject()); }

	/** 장착/해제 토글 요청 (bIsCurrentlyEquipped: 현재 장착 상태) */
	void RequestToggle(bool bIsCurrentlyEquipped);

	/** 몽타주 정상 종료 시 호출 (bNewEquippedState: 전환 후 상태) */
	void OnMontageFinished(bool bNewEquippedState);

	/** 몽타주 중단 시 호출 (전환 취소) */
	void OnMontageInterrupted();

	/** 무기를 캐릭터 메시에 부착 */
	void AttachWeapon();

	/** 무기를 캐릭터 메시에서 분리 */
	void DetachWeapon();

	FOnEquipMontagePlay OnEquipMontagePlayDelegate;
	FOnEquipStateChanged OnEquipStateChangedDelegate;

protected:
	/** 현재 무기 인터페이스 (IAttachable) */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TScriptInterface<IAttachable> WeaponInterface;

	/** 장착/해제 전환 중 플래그 (중복 요청 방지) */
	uint8 bIsEquipTransitioning : 1 = false;
};