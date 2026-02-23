#include "Character/Component/Action/TPSCMC.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

DECLARE_LOG_CATEGORY_EXTERN(SprintSpeedTickLog, Warning, All);

DEFINE_LOG_CATEGORY(SprintSpeedTickLog);

UTPSCMC::UTPSCMC()
{
	InitTickFunctions();
}


void UTPSCMC::InitTickFunctions()
{
	InterpolateTickFunction.bCanEverTick = true;
	InterpolateTickFunction.bStartWithTickEnabled = false;
	InterpolateTickFunction.TickGroup = TG_PrePhysics;
}

void UTPSCMC::RegisterComponentTickFunctions(bool bRegister)
{
	Super::RegisterComponentTickFunctions(bRegister);

	if (bRegister)
	{
		InterpolateTickFunction.Target = this;
		InterpolateTickFunction.RegisterTickFunction(GetComponentLevel());
		InterpolateTickFunction.SetTickFunctionEnable(InterpolateTickFunction.bStartWithTickEnabled);
	}
	else
	{
		if (InterpolateTickFunction.IsTickFunctionRegistered())
		{
			InterpolateTickFunction.UnRegisterTickFunction();
		}
	}
}

void UTPSCMC::UpdateSprintSpeed(float TargetSpeed)
{
	InterpTargetSpeed = TargetSpeed;
	SetInterpolateTickEnabled(true);
}

void UTPSCMC::SetInterpolateTickEnabled(bool bEnabled)
{
	if (InterpolateTickFunction.IsTickFunctionEnabled() == bEnabled) return;

	InterpolateTickFunction.SetTickFunctionEnable(bEnabled);
	UE_LOG(SprintSpeedTickLog, Warning, TEXT("[SetInterpolateTickEnabled] %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void UTPSCMC::Interpolate_Tick(float DeltaTime)
{
	MaxWalkSpeed = FMath::FInterpTo(MaxWalkSpeed, InterpTargetSpeed, DeltaTime, SpeedInterpRate);

	if (FMath::IsNearlyEqual(MaxWalkSpeed, InterpTargetSpeed, 1.f))
	{
		MaxWalkSpeed = InterpTargetSpeed;
		SetInterpolateTickEnabled(false);
	}

#if !UE_BUILD_SHIPPING
	GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, FString::Printf(TEXT("Speed: %.1f"), MaxWalkSpeed));
#endif
}
