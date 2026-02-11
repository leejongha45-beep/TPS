#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPSPlayerStatusComponent.generated.h"


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
	UPROPERTY(VisibleDefaultsOnly, Category="Speed")
	float DefaultWalkSpeed = 0.f;

	UPROPERTY(VisibleDefaultsOnly, Category="Speed")
	float DefaultSprintSpeed = 0.f;
};