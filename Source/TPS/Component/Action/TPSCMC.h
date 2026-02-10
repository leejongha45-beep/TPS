#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TPSCMC.generated.h"

/**
 * 
 */
UCLASS()
class TPS_API UTPSCMC : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
	
public:
	FORCEINLINE void SetOrientRotationToMovement(bool bInput) {bOrientRotationToMovement = bInput;}
	
	FORCEINLINE void SetMaxWalkSpeed(int32 InputSpeed) {MaxWalkSpeed = InputSpeed;}
};