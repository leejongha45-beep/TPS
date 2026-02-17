#include "Core/GameMode/TPSGameModeBase.h"

#include "Core/Controller/TPSPlayerController.h"
#include "Pawn/Character/Player/TPSPlayer.h"
#include "UI/HUD/TPSHUD.h"

ATPSGameModeBase::ATPSGameModeBase()
{
	DefaultPawnClass = BP_PlayerClass;

	PlayerControllerClass = BP_PlayerControllerClass;

	HUDClass = BP_HUDClass;
}