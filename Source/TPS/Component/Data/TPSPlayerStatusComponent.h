#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPSPlayerStatusComponent.generated.h"


/**
 * 플레이어 스탯 데이터 컴포넌트
 * - 성장 시스템과 연동될 기본 수치 저장
 * - 걷기/달리기 기본 속도 등 캐릭터 스탯
 * - FORCEINLINE Setter/Getter로 캡슐화
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TPS_API UTPSPlayerStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FORCEINLINE void SetDefaultWalkSpeed(float InputSpeed) { DefaultWalkSpeed = InputSpeed; }
	FORCEINLINE void SetDefaultSprintSpeed(float InputSpeed) { DefaultSprintSpeed = InputSpeed; }

	FORCEINLINE float GetDefaultWalkSpeed() const { return DefaultWalkSpeed; }
	FORCEINLINE float GetDefaultSprintSpeed() const { return DefaultSprintSpeed; }

protected:
	/** 기본 걷기 속도 (CMC에서 초기화) */
	UPROPERTY(VisibleDefaultsOnly, Category="Speed")
	float DefaultWalkSpeed = 0.f;

	/** 기본 달리기 속도 */
	UPROPERTY(VisibleDefaultsOnly, Category="Speed")
	float DefaultSprintSpeed = 0.f;
};