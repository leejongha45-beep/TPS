#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utils/Interface/Data/Interpolable.h"
#include "Utils/TickFunctions/FInterpolateTickFunction.h"
#include "TPSCameraControlComponent.generated.h"

/**
 * 카메라 제어 컴포넌트
 * - ADS(Aim Down Sight) 전환 시 FOV, SocketOffset, ArmLength 보간
 * - IInterpolable 구현 — FInterpolateTickFunction으로 부드러운 전환
 * - Player의 SpringArm + Camera 참조를 Initialize()에서 받음
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TPS_API UTPSCameraControlComponent : public UActorComponent, public IInterpolable
{
	GENERATED_BODY()

public:
	UTPSCameraControlComponent();

	/** SpringArm/Camera 참조 초기화 (Player::BeginPlay에서 호출) */
	void Initialize(class USpringArmComponent* InSpringArm, class UCameraComponent* InCamera);

	/** ADS 모드 진입 */
	void StartADS();

	/** ADS 모드 해제 */
	void StopADS();

	FORCEINLINE bool IsADS() const { return bIsADS; }

protected:
	virtual void RegisterComponentTickFunctions(bool bRegister) override;
	void InitTickFunctions();

	/** IInterpolable — 매 프레임 카메라 보간 */
	virtual void Interpolate_Tick(float DeltaTime) override;
	void SetInterpolateTickEnabled(bool bEnabled);

	/** 보간 전용 Tick 함수 */
	FInterpolateTickFunction InterpolateTickFunction;

	/** SpringArm 참조 (비소유) */
	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Camera")
	TObjectPtr<class USpringArmComponent> SpringArmRef;

	/** Camera 참조 (비소유) */
	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Camera")
	TObjectPtr<class UCameraComponent> CameraRef;

	uint8 bIsADS : 1 = false;

#pragma region ADS Settings
	/** 기본 시야각 */
	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	float DefaultFOV = 90.f;

	/** ADS 시야각 */
	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	float ADSFOV = 65.f;

	/** 기본 소켓 오프셋 */
	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	FVector DefaultSocketOffset = FVector(0.f, 50.f, 40.f);

	/** ADS 소켓 오프셋 */
	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	FVector ADSSocketOffset = FVector(0.f, 60.f, 15.f);

	/** 기본 스프링암 길이 */
	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	float DefaultArmLength = 300.f;

	/** ADS 스프링암 길이 */
	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	float ADSArmLength = 150.f;

	/** ADS 보간 속도 */
	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	float ADSInterpRate = 10.f;
#pragma endregion

	float TargetFOV;
	FVector TargetSocketOffset;
	float TargetArmLength;
};