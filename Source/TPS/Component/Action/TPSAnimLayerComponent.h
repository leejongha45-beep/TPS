#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPSAnimLayerComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TPS_API UTPSAnimLayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTPSAnimLayerComponent();

	UFUNCTION(BlueprintCallable, Category="Animation")
	void LinkAnimLayer(TSubclassOf<UTPSLinkedAnimInstance> InClass);

	UFUNCTION(BlueprintCallable, Category="Animation")
	void UnlinkAnimLayer();

	FORCEINLINE TSubclassOf<class UTPSLinkedAnimInstance> GetUnArmedLayerClass() const { return UnArmedAnimLayerClass; }
	FORCEINLINE TSubclassOf<class UTPSLinkedAnimInstance> GetDefaultLayerClass() const { return DefaultAnimLayerClass; }
	FORCEINLINE TSubclassOf<class UTPSLinkedAnimInstance> GetRifleHipFireLayerClass() const { return RifleHipFireAnimLayerClass; }
	FORCEINLINE TSubclassOf<class UTPSLinkedAnimInstance> GetRifleADSLayerClass() const { return RifleADSAnimLayerClass; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TSubclassOf<class UTPSLinkedAnimInstance> UnArmedAnimLayerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TSubclassOf<class UTPSLinkedAnimInstance> DefaultAnimLayerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TSubclassOf<class UTPSLinkedAnimInstance> RifleHipFireAnimLayerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TSubclassOf<class UTPSLinkedAnimInstance> RifleADSAnimLayerClass;

	UPROPERTY(Transient)
	TSubclassOf<class UTPSLinkedAnimInstance> CurrentAnimLayerClass;
};