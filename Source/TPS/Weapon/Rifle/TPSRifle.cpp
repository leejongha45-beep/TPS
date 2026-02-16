#include "TPSRifle.h"

ATPSRifle::ATPSRifle()
{
	FireInterval = 0.1f;
	MaxAmmo = 60;
	CurrentAmmo = 60;
	WeaponDamage = 20.f;
	ProjectileSpeed = 10000.f;
	ReloadTime = 2.5f;
}
