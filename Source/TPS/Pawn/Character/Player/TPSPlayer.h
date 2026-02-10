#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Utils/Interface/Action/Moveable.h"
#include "TPSPlayer.generated.h"

UCLASS()
class TPS_API ATPSPlayer : public ACharacter, public IMoveable
{
	GENERATED_BODY()

public:
	ATPSPlayer(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void PostInitializeComponents() override;

	void CreateDefaultComponents();

#pragma region Component

	UPROPERTY(VisibleDefaultsOnly, Category = "Camera")
	TObjectPtr<class USpringArmComponent> SpringArmComponentInst;

	UPROPERTY(VisibleDefaultsOnly, Category = "Camera")
	TObjectPtr<class UCameraComponent> CameraComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UTPSCMC> CachedCMC;
#pragma endregion

	virtual void Move(const FVector2D& InputVector) override;
};