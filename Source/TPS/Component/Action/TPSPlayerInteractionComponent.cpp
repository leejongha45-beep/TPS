#include "Component/Action/TPSPlayerInteractionComponent.h"
#include "Blueprint/UserWidget.h"
#include "Component/Data/TPSPlayerStateComponent.h"
#include "Pawn/Character/Player/TPSPlayer.h"
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
		if (bIsInteracting)
		{
			CloseInteraction();
		}

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

	// 상호작용할 액터의 Interact호출 함으로써 결합도 완화
	pInteractable->Interact();

	HidePrompt();

	ATPSPlayer* pPlayer = Cast<ATPSPlayer>(GetOwner());
	if (!ensure(pPlayer)) return;

	UTPSPlayerStateComponent* pStateComponent = pPlayer->GetStateComponent();
	if (ensure(pStateComponent))
	{
		pStateComponent->AddState(EActionState::Interacting);
	}

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

	IInteractable* pInteractable = Cast<IInteractable>(pTargetActor);
	if (ensure(pInteractable))
	{
		pInteractable->Interact();
	}

	ATPSPlayer* pPlayer = Cast<ATPSPlayer>(GetOwner());
	if (!ensure(pPlayer)) return;

	UTPSPlayerStateComponent* pStateComponent = pPlayer->GetStateComponent();
	if (ensure(pStateComponent))
	{
		pStateComponent->RemoveState(EActionState::Interacting);
	}

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

	ATPSPlayer* pPlayer = Cast<ATPSPlayer>(GetOwner());
	if (!ensure(pPlayer)) return;

	APlayerController* pController = Cast<APlayerController>(pPlayer->GetController());
	if (!ensure(pController)) return;
	if (!ensure(InteractionPromptWidgetClass)) return;

	InteractionPromptWidgetInst = CreateWidget<UTPSInteractionPromptWidget>(pController, InteractionPromptWidgetClass);
	if (ensure(InteractionPromptWidgetInst))
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