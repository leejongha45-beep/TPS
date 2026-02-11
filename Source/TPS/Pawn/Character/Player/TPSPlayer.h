#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Utils/Interface/Action/Aimable.h"
#include "Utils/Interface/Action/Moveable.h"
#include "Utils/Interface/Action/Sprintable.h"
#include "TPSPlayer.generated.h"

UCLASS()
class TPS_API ATPSPlayer : public ACharacter, public IMoveable, public ISprintable, public IAimable
{
	GENERATED_BODY()

public:
	ATPSPlayer(const FObjectInitializer& ObjectInitializer);

	FORCEINLINE class UTPSPlayerStateComponent* GetStateComponent() const { return StateComponentInst; }

protected:
	virtual void PostInitializeComponents() override;

	void CreateDefaultComponents();

#pragma region Component

	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Camera")
	TObjectPtr<class USpringArmComponent> SpringArmComponentInst;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Camera")
	TObjectPtr<class UCameraComponent> CameraComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Action")
	TObjectPtr<class UTPSCMC> CachedCMC;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|State")
	TObjectPtr<class UTPSPlayerStateComponent> StateComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Status")
	TObjectPtr<class UTPSPlayerStatusComponent> StatusComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Camera")
	TObjectPtr<class UTPSCameraControlComponent> CameraControlComponentInst;
#pragma endregion

	virtual void StartMove() override;
	virtual void Move(const FVector2D& InputVector) override;
	virtual void StopMove() override;

	virtual void StartSprint() override;
	virtual void StopSprint() override;

	virtual void StartAim() override;
	virtual void StopAim() override;
};