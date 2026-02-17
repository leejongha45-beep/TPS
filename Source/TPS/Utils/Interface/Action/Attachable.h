#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Attachable.generated.h"

UINTERFACE()
class UAttachable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 부착/분리 인터페이스
 * - 무기 등 액터를 SkeletalMesh 소켓에 부착/분리
 * - TPSWeaponBase가 구현하여 장착 시스템과 연동
 */
class TPS_API IAttachable
{
	GENERATED_BODY()

public:
	/** 대상 메시의 소켓에 부착 */
	virtual void Attach(USkeletalMeshComponent* InTargetMesh) = 0;

	/** 대상 메시에서 분리 */
	virtual void Detach(USkeletalMeshComponent* InTargetMesh) = 0;
};
