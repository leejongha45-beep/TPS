#include "TPSPlayerStateComponent.h"

DEFINE_LOG_CATEGORY(StateLog);

void UTPSPlayerStateComponent::AddState(EActionState InState)
{
	CurrentState |= InState;
	UE_LOG(StateLog, Log, TEXT("[AddState] Added: %s"), *UEnum::GetValueAsString(InState));
}

void UTPSPlayerStateComponent::RemoveState(EActionState InState)
{
	CurrentState &= ~InState;
	UE_LOG(StateLog, Log, TEXT("[RemoveState] Removed: %s"), *UEnum::GetValueAsString(InState));
}

void UTPSPlayerStateComponent::ClearState()
{
	UE_LOG(StateLog, Log, TEXT("[ClearState] State Cleared"));
	CurrentState = EActionState::None;
}
