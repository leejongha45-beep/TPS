#include "Character/Component/Action/TPSPlayerInteractionComponent.h"
#include "Blueprint/UserWidget.h"
#include "Character/Component/Data/TPSPlayerStateComponent.h"
#include "Character/Player/TPSPlayer.h"
#include "UI/Widget/Interaction/TPSInteractionPromptWidget.h"
#include "Utils/Interface/Action/Interactable.h"

void UTPSPlayerInteractionComponent::HandleInteraction()
{
	if (!CurrentTargetRef.IsValid()) return;

	if (bIsInteracting)
	{
		CloseInteraction();
	}
	else
	{
		OpenInteraction();
	}
}

void UTPSPlayerInteractionComponent::SetCurrentTarget(AActor* InTarget)
{
	if (!ensure(InTarget)) return;

	CurrentTargetRef = InTarget;
	ShowPrompt();
}

void UTPSPlayerInteractionComponent::ClearCurrentTarget(AActor* InTarget)
{
	if (!ensure(InTarget)) return;

	if (CurrentTargetRef.Get() == InTarget)
	{
		// ① 상호작용 중이면 먼저 닫기
		if (bIsInteracting)
		{
			CloseInteraction();
		}

		// ② 프롬프트 제거 + 참조 초기화
		HidePrompt();
		CurrentTargetRef.Reset();
	}
}

void UTPSPlayerInteractionComponent::OpenInteraction()
{
	AActor* pTargetActor = CurrentTargetRef.Get();
	if (!ensure(pTargetActor)) return;

	IInteractable* pInteractable = Cast<IInteractable>(pTargetActor);
	if (!ensure(pInteractable)) return;

	// ① 대상 액터의 Interact 호출 (결합도 완화 — 인터페이스 경유)
	pInteractable->Interact();

	// ② 프롬프트 숨김
	HidePrompt();

	// ③ 플레이어 상태에 Interacting 추가
	ATPSPlayer* pPlayer = Cast<ATPSPlayer>(GetOwner());
	if (!ensure(pPlayer)) return;

	UTPSPlayerStateComponent* pStateComponent = pPlayer->GetStateComponent();
	if (ensure(pStateComponent))
	{
		pStateComponent->AddState(EActionState::Interacting);
	}

	// ④ 입력 모드 → GameAndUI + 마우스 커서 표시
	APlayerController* pController = Cast<APlayerController>(pPlayer->GetController());
	if (ensure(pController))
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		pController->SetInputMode(InputMode);
		pController->SetShowMouseCursor(true);
	}

	bIsInteracting = true;
}

void UTPSPlayerInteractionComponent::CloseInteraction()
{
	AActor* pTargetActor = CurrentTargetRef.Get();
	if (!ensure(pTargetActor))
	{
		bIsInteracting = false;
		return;
	}

	// ① 대상 액터의 Interact 재호출 (토글 닫기)
	IInteractable* pInteractable = Cast<IInteractable>(pTargetActor);
	if (ensure(pInteractable))
	{
		pInteractable->Interact();
	}

	// ② 플레이어 상태에서 Interacting 제거
	ATPSPlayer* pPlayer = Cast<ATPSPlayer>(GetOwner());
	if (!ensure(pPlayer)) return;

	UTPSPlayerStateComponent* pStateComponent = pPlayer->GetStateComponent();
	if (ensure(pStateComponent))
	{
		pStateComponent->RemoveState(EActionState::Interacting);
	}

	// ③ 입력 모드 → GameOnly + 마우스 커서 숨김
	APlayerController* pController = Cast<APlayerController>(pPlayer->GetController());
	if (ensure(pController))
	{
		pController->SetInputMode(FInputModeGameOnly());
		pController->SetShowMouseCursor(false);
	}

	bIsInteracting = false;
}

void UTPSPlayerInteractionComponent::ShowPrompt()
{
	if (InteractionPromptWidgetInst) return;

	// ① 플레이어 → 컨트롤러 참조 획득
	ATPSPlayer* pPlayer = Cast<ATPSPlayer>(GetOwner());
	if (!ensure(pPlayer)) return;

	APlayerController* pController = Cast<APlayerController>(pPlayer->GetController());
	if (!ensure(pController)) return;
	if (!ensure(InteractionPromptWidgetClass.Get())) return;

	// ② 위젯 생성 → 텍스트 설정 → 뷰포트 추가
	InteractionPromptWidgetInst = CreateWidget<UTPSInteractionPromptWidget>(pController, InteractionPromptWidgetClass);
	if (ensure(InteractionPromptWidgetInst.Get()))
	{
		InteractionPromptWidgetInst->SetPromptText(InteractionPromptText);
		InteractionPromptWidgetInst->AddToViewport();
	}
}

void UTPSPlayerInteractionComponent::HidePrompt()
{
	if (InteractionPromptWidgetInst)
	{
		InteractionPromptWidgetInst->RemoveFromParent();
		InteractionPromptWidgetInst = nullptr;
	}
}