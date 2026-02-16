#include "Core/GameMode/TPSGameModeBase.h"

#include "Core/Controller/TPSPlayerController.h"
#include "GameFramework/HUD.h"
#include "Pawn/Character/Player/TPSPlayer.h"

ATPSGameModeBase::ATPSGameModeBase()
{
	if (BP_PlayerClass)
	{
		DefaultPawnClass = BP_PlayerClass;
	}

	if (BP_PlayerControllerClass)
	{
		PlayerControllerClass = BP_PlayerControllerClass;
	}
	
	if (BP_HUDClass)
	{
		HUDClass = BP_HUDClass;
	}
}