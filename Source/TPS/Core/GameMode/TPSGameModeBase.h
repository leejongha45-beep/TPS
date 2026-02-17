#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TPSGameModeBase.generated.h"

UCLASS()
class TPS_API ATPSGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATPSGameModeBase();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Classes")
	TSubclassOf<class ATPSPlayer> BP_PlayerClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Classes")
	TSubclassOf<class ATPSPlayerController> BP_PlayerControllerClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Classes")
	TSubclassOf<class ATPSHUD> BP_HUDClass;
};