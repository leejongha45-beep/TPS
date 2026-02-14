#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPSPlayerInteractionComponent.generated.h"

UCLASS()
class TPS_API UTPSPlayerInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void HandleInteraction();

	void SetCurrentTarget(AActor* InTarget);
	void ClearCurrentTarget(AActor* InTarget);

	void CloseInteraction();

	FORCEINLINE AActor* GetCurrentTarget() const { return CurrentTargetRef.Get(); }
	FORCEINLINE bool GetIsInteracting() const { return bIsInteracting; }

protected:
	void OpenInteraction();

	void ShowPrompt();
	void HidePrompt();

	TWeakObjectPtr<AActor> CurrentTargetRef;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Interaction")
	TSubclassOf<class UTPSInteractionPromptWidget> InteractionPromptWidgetClass;

	UPROPERTY()
	TObjectPtr<class UTPSInteractionPromptWidget> InteractionPromptWidgetInst;

	uint8 bIsInteracting : 1 = false;
};
