#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPSPlayerInteractionComponent.generated.h"

/**
 * 플레이어 상호작용 컴포넌트
 * - 오버랩 감지된 대상(ItemBox 등)과의 상호작용 관리
 * - 프롬프트 위젯 표시/숨김
 * - HandleInteraction: 토글 방식 (Open ↔ Close)
 */
UCLASS()
class TPS_API UTPSPlayerInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** 상호작용 토글 (Open ↔ Close) */
	void HandleInteraction();

	/** 오버랩 진입 시 대상 설정 */
	void SetCurrentTarget(AActor* InTarget);

	/** 오버랩 이탈 시 대상 해제 */
	void ClearCurrentTarget(AActor* InTarget);

	/** 상호작용 종료 (외부에서 강제 닫기) */
	void CloseInteraction();

	FORCEINLINE AActor* GetCurrentTarget() const { return CurrentTargetRef.Get(); }
	FORCEINLINE bool GetIsInteracting() const { return bIsInteracting; }

protected:
	/** 상호작용 열기 (UI 등 활성화) */
	void OpenInteraction();

	/** 프롬프트 위젯 표시 */
	void ShowPrompt();

	/** 프롬프트 위젯 숨김 */
	void HidePrompt();

	/** 현재 상호작용 대상 (WeakPtr) */
	TWeakObjectPtr<AActor> CurrentTargetRef;

	/** 프롬프트 위젯 BP 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "UI|Interaction")
	TSubclassOf<class UTPSInteractionPromptWidget> InteractionPromptWidgetClass;

	/** 프롬프트 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<class UTPSInteractionPromptWidget> InteractionPromptWidgetInst;

	uint8 bIsInteracting : 1 = false;

	/** 프롬프트 표시 텍스트 */
	UPROPERTY(EditDefaultsOnly, Category = "UI|Interaction")
	FText InteractionPromptText = FText::FromString(TEXT("Press F to Interact"));
};
