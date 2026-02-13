#include "Component/Action/TPSCameraControlComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

DECLARE_LOG_CATEGORY_EXTERN(CameraControlLog, Warning, All);

DEFINE_LOG_CATEGORY(CameraControlLog);

UTPSCameraControlComponent::UTPSCameraControlComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	TargetFOV = DefaultFOV;
	TargetSocketOffset = DefaultSocketOffset;
	TargetArmLength = DefaultArmLength;

	InitTickFunctions();
}

void UTPSCameraControlComponent::InitTickFunctions()
{
	InterpolateTickFunction.bCanEverTick = true;
	InterpolateTickFunction.bStartWithTickEnabled = false;
	InterpolateTickFunction.TickGroup = TG_PostPhysics;
}

void UTPSCameraControlComponent::RegisterComponentTickFunctions(bool bRegister)
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
		InterpolateTickFunction.UnRegisterTickFunction();
	}
}

void UTPSCameraControlComponent::Initialize(USpringArmComponent* InSpringArm, UCameraComponent* InCamera)
{
	SpringArmRef = InSpringArm;
	CameraRef = InCamera;

	if (ensure(SpringArmRef))
	{
		SpringArmRef->SocketOffset = DefaultSocketOffset;
		SpringArmRef->TargetArmLength = DefaultArmLength;
	}

	if (ensure(CameraRef))
	{
		CameraRef->SetFieldOfView(DefaultFOV);
	}
}

void UTPSCameraControlComponent::StartADS()
{
	bIsADS = true;
	TargetFOV = ADSFOV;
	TargetSocketOffset = ADSSocketOffset;
	TargetArmLength = ADSArmLength;
	SetInterpolateTickEnabled(true);

	UE_LOG(CameraControlLog, Warning, TEXT("[StartADS] ADS Enabled"));
}

void UTPSCameraControlComponent::StopADS()
{
	bIsADS = false;
	TargetFOV = DefaultFOV;
	TargetSocketOffset = DefaultSocketOffset;
	TargetArmLength = DefaultArmLength;
	SetInterpolateTickEnabled(true);

	UE_LOG(CameraControlLog, Warning, TEXT("[StopADS] ADS Disabled"));
}

void UTPSCameraControlComponent::SetInterpolateTickEnabled(bool bEnabled)
{
	if (InterpolateTickFunction.IsTickFunctionEnabled() == bEnabled) return;

	InterpolateTickFunction.SetTickFunctionEnable(bEnabled);
	UE_LOG(CameraControlLog, Warning, TEXT("[SetInterpolateTickEnabled] %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void UTPSCameraControlComponent::Interpolate_Tick(float DeltaTime)
{
	bool bFinished_C = false;
	bool bFinished_S = false;

	if (ensure(CameraRef))
	{
		float CurrentFOV = CameraRef->FieldOfView;
		float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, ADSInterpRate);
		CameraRef->SetFieldOfView(NewFOV);

		if (FMath::IsNearlyEqual(NewFOV, TargetFOV, 0.1f))
		{
			bFinished_C = true;
		}
	}

	if (ensure(SpringArmRef))
	{
		FVector CurrentOffset = SpringArmRef->SocketOffset;
		FVector NewOffset = FMath::VInterpTo(CurrentOffset, TargetSocketOffset, DeltaTime, ADSInterpRate);
		SpringArmRef->SocketOffset = NewOffset;

		float CurrentLength = SpringArmRef->TargetArmLength;
		float NewLength = FMath::FInterpTo(CurrentLength, TargetArmLength, DeltaTime, ADSInterpRate);
		SpringArmRef->TargetArmLength = NewLength;

		if (NewOffset.Equals(TargetSocketOffset, 0.1f) && FMath::IsNearlyEqual(NewLength, TargetArmLength, 0.1f))
		{
			bFinished_S = true;
		}
	}

	if (bFinished_C && bFinished_S)
	{
		if (ensure(CameraRef))
		{
			CameraRef->SetFieldOfView(TargetFOV);
		}
		
		if (ensure(SpringArmRef))
		{
			SpringArmRef->SocketOffset = TargetSocketOffset;
			SpringArmRef->TargetArmLength = TargetArmLength;
		}
		
		SetInterpolateTickEnabled(false);
	}
}