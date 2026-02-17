#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Utils/UENUM/SurfaceType.h"
#include "FootstepData.generated.h"

USTRUCT(BlueprintType)
struct FFootstepSoundRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Footstep")
	ESurfaceType SurfaceType = ESurfaceType::Default;

	UPROPERTY(EditDefaultsOnly, Category = "Footstep|Walk")
	TArray<TObjectPtr<class USoundBase>> WalkSoundAssets;

	UPROPERTY(EditDefaultsOnly, Category = "Footstep|Run")
	TArray<TObjectPtr<class USoundBase>> RunSoundAssets;

	UPROPERTY(EditDefaultsOnly, Category = "Footstep|Sound", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float VolumeMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Footstep|Sound", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float PitchVariation = 0.1f;
};
