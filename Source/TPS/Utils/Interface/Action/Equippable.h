#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Equippable.generated.h"

UINTERFACE()
class UEquippable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 장착/해제 액션 인터페이스
 * - Controller → Player(Character) 장착 명령 전달
 * - EActionState::Equipping 플래그와 연동
 * - TPSEquipComponent에서 몽타주 재생 + AnimLayer 전환
 */
class TPS_API IEquippable
{
	GENERATED_BODY()

public:
	/** 무기 장착 시작 (장착 몽타주 재생) */
	virtual void Equip() = 0;

	/** 무기 해제 시작 (해제 몽타주 재생) */
	virtual void Unequip() = 0;
};
