#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPSAnimLayerComponent.generated.h"

/**
 * 애니메이션 레이어 관리 컴포넌트
 * - Lyra 스타일 Animation Layer Interface (ALI) 시스템
 * - 무기/상태에 따라 LinkedAnimInstance를 동적으로 Link/Unlink
 * - 장착 상태: UnArmed → HipFire → ADS 전환
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TPS_API UTPSAnimLayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTPSAnimLayerComponent();

	/** 애님 레이어 링크 (LinkedAnimInstance BP 클래스 전달) */
	UFUNCTION(BlueprintCallable, Category="Animation")
	void LinkAnimLayer(TSubclassOf<UTPSLinkedAnimInstance> InClass);

	/** 현재 링크된 애님 레이어 해제 */
	UFUNCTION(BlueprintCallable, Category="Animation")
	void UnlinkAnimLayer();

	FORCEINLINE TSubclassOf<class UTPSLinkedAnimInstance> GetUnArmedLayerClass() const { return UnArmedAnimLayerClass; }
	FORCEINLINE TSubclassOf<class UTPSLinkedAnimInstance> GetDefaultLayerClass() const { return DefaultAnimLayerClass; }
	FORCEINLINE TSubclassOf<class UTPSLinkedAnimInstance> GetRifleHipFireLayerClass() const { return RifleHipFireAnimLayerClass; }
	FORCEINLINE TSubclassOf<class UTPSLinkedAnimInstance> GetRifleADSLayerClass() const { return RifleADSAnimLayerClass; }

protected:
	/** 비무장 레이어 (무기 미장착 시) */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TSubclassOf<class UTPSLinkedAnimInstance> UnArmedAnimLayerClass;

	/** 기본 레이어 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TSubclassOf<class UTPSLinkedAnimInstance> DefaultAnimLayerClass;

	/** 라이플 힙파이어 레이어 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TSubclassOf<class UTPSLinkedAnimInstance> RifleHipFireAnimLayerClass;

	/** 라이플 ADS 레이어 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TSubclassOf<class UTPSLinkedAnimInstance> RifleADSAnimLayerClass;

	/** 현재 링크된 레이어 (Transient — 저장 안 함) */
	UPROPERTY(Transient)
	TSubclassOf<class UTPSLinkedAnimInstance> CurrentAnimLayerClass;
};