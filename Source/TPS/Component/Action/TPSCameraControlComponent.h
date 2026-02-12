#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utils/Interface/Data/Interpolable.h"
#include "Utils/TickFunctions/FInterpolateTickFunction.h"
#include "TPSCameraControlComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TPS_API UTPSCameraControlComponent : public UActorComponent, public IInterpolable
{
	GENERATED_BODY()

public:
	UTPSCameraControlComponent();

	void Initialize(class USpringArmComponent* InSpringArm, class UCameraComponent* InCamera);

	void StartADS();
	void StopADS();

	FORCEINLINE bool IsADS() const { return bIsADS; }

protected:
	virtual void RegisterComponentTickFunctions(bool bRegister) override;

	void InitTickFunctions();

	virtual void Interpolate_Tick(float DeltaTime) override;
	void SetInterpolateTickEnabled(bool bEnabled);

	FInterpolateTickFunction InterpolateTickFunction;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Camera")
	TObjectPtr<class USpringArmComponent> SpringArmRef;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Camera")
	TObjectPtr<class UCameraComponent> CameraRef;

	uint8 bIsADS : 1 = false;

#pragma region ADS Settings
	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	float DefaultFOV = 90.f;

	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	float ADSFOV = 65.f;

	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	FVector DefaultSocketOffset = FVector(0.f, 50.f, 40.f);

	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	FVector ADSSocketOffset = FVector(0.f, 60.f, 15.f);

	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	float DefaultArmLength = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	float ADSArmLength = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	float ADSInterpRate = 10.f;
#pragma endregion

	float TargetFOV;
	FVector TargetSocketOffset;
	float TargetArmLength;
};