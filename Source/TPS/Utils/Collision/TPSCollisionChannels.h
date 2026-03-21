#pragma once

#include "Engine/EngineTypes.h"

/** 커스텀 콜리전 채널 — DefaultEngine.ini에 정의 */
namespace TPSCollision
{
	inline constexpr ECollisionChannel Enemy      = ECC_GameTraceChannel1;
	inline constexpr ECollisionChannel Projectile = ECC_GameTraceChannel2;
}
