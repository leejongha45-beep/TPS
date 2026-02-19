#include "UI/ViewModel/CrosshairViewModel.h"
#include "Component/Data/TPSPlayerStateComponent.h"
#include "Pawn/Character/Player/TPSPlayer.h"

void UCrosshairViewModel::Initialize(TObjectPtr<ATPSPlayer> InPlayer, const FCrosshairConfig& InConfig)
{
	Config = InConfig;
	PlayerRef = InPlayer;

	if (ensure(PlayerRef.Get()))
	{
		StateComponentRef = PlayerRef.Get()->GetStateComponent();
		ensure(StateComponentRef.Get());
	}

	CurrentSpreadOffset = Config.BaseSpreadOffset;
}

void UCrosshairViewModel::Update(float DeltaTime)
{
	if (!PlayerRef.IsValid() || !StateComponentRef.IsValid())
	{
		bIsVisible = false;
		return;
	}

	UTPSPlayerStateComponent* pStateComp = StateComponentRef.Get();
	if (!ensure(pStateComp))
	{
		bIsVisible = false;
		return;
	}

	// ① 비장착 상태 → 크로스헤어 숨김
	if (!pStateComp->HasState(EActionState::Equipping))
	{
		bIsVisible = false;
		return;
	}

	// ② 목표 스프레드 계산 → 현재 값 보간
	bIsVisible = true;
	TargetSpreadOffset = CalculateTargetSpread();
	CurrentSpreadOffset = FMath::FInterpTo(CurrentSpreadOffset, TargetSpreadOffset, DeltaTime, Config.SpreadInterpSpeed);
}

float UCrosshairViewModel::CalculateTargetSpread() const
{
	UTPSPlayerStateComponent* pStateComp = StateComponentRef.Get();
	if (!ensure(pStateComp))
	{
		return Config.BaseSpreadOffset;
	}

	// ① 기본값 + 상태별 가산 (Moving, Sprinting, Airborne, Firing)
	float Spread = Config.BaseSpreadOffset;

	if (pStateComp->HasState(EActionState::Moving))
	{
		Spread += Config.MovingSpreadAdditive;
	}

	if (pStateComp->HasState(EActionState::Sprinting))
	{
		Spread += Config.SprintingSpreadAdditive;
	}

	if (pStateComp->HasState(EActionState::Jumping) || pStateComp->HasState(EActionState::Falling))
	{
		Spread += Config.AirborneSpreadAdditive;
	}

	if (pStateComp->HasState(EActionState::Firing))
	{
		Spread += Config.FiringSpreadAdditive;
	}

	// ② 조준 시 최종 곱수 적용
	if (pStateComp->HasState(EActionState::Aiming))
	{
		Spread *= Config.AimingSpreadMultiplier;
	}

	return Spread;
}