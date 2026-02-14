#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Utils/Interface/Data/Interpolable.h"
#include "Utils/TickFunctions/FInterpolateTickFunction.h"
#include "TPSCMC.generated.h"

UCLASS()
class TPS_API UTPSCMC : public UCharacterMovementComponent, public IInterpolable
{
	GENERATED_BODY()

public:
	UTPSCMC();

	FORCEINLINE void SetOrientRotationToMovement(bool bInput) { bOrientRotationToMovement = bInput; }
	
	FORCEINLINE void SetRotationRate(const FRotator& InputRate) { RotationRate = InputRate; }

	FORCEINLINE void SetMaxWalkSpeed(int32 InputSpeed) { MaxWalkSpeed = InputSpeed; }

protected:
	virtual void RegisterComponentTickFunctions(bool bRegister) override;

	void InitTickFunctions();

#pragma region Sprint

public:
	void UpdateSprintSpeed(float TargetSpeed);

protected:
	FInterpolateTickFunction InterpolateTickFunction;

	virtual void Interpolate_Tick(float DeltaTime) override;
	void SetInterpolateTickEnabled(bool bEnabled);

	float InterpTargetSpeed = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Rate")
	float SpeedInterpRate = 5.f;
#pragma endregion
};