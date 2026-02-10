#include "Pawn/Character/Player/TPSPlayer.h"
#include "Component/Action/TPSCMC.h"

ATPSPlayer::ATPSPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UTPSCMC>(ACharacter::CharacterMovementComponentName))
{
}