#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utils/UENUM/State.h"
#include "TPSPlayerStateComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(StateLog, Log, All);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TPS_API UTPSPlayerStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void AddState(EActionState InState);
	void RemoveState(EActionState InState);
	void ClearState();
	
	FORCEINLINE bool HasState(EActionState InState) const { return EnumHasAnyFlags(CurrentState, InState); }
	FORCEINLINE EActionState GetCurrentState() const { return CurrentState; }

protected:
	EActionState CurrentState = EActionState::None;
};