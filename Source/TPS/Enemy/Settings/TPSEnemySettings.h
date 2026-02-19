#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TPSEnemySettings.generated.h"

/**
 * 적 시스템 전역 설정
 * Project Settings -> Game -> TPS Enemy Settings
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="TPS Enemy Settings"))
class TPS_API UTPSEnemySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UTPSEnemySettings();

	/** ISM 렌더링에 사용할 StaticMesh */
	UPROPERTY(EditDefaultsOnly, Config, Category = "ISM")
	TSoftObjectPtr<UStaticMesh> EnemyISMMesh;

	virtual FName GetCategoryName() const override { return TEXT("Game"); }
};
