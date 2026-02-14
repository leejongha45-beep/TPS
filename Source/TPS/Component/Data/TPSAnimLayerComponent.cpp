#include "Component/Data/TPSAnimLayerComponent.h"
#include "Animation/Player/TPSLinkedAnimInstance.h"
#include "GameFramework/Character.h"

UTPSAnimLayerComponent::UTPSAnimLayerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTPSAnimLayerComponent::LinkAnimLayer(TSubclassOf<UTPSLinkedAnimInstance> InClass)
{
	if (!InClass || InClass == CurrentAnimLayerClass) return;

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!ensure(Character)) return;

	Character->GetMesh()->LinkAnimClassLayers(InClass);
	CurrentAnimLayerClass = InClass;
}

void UTPSAnimLayerComponent::UnlinkAnimLayer()
{
	if (!CurrentAnimLayerClass) return;

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!ensure(Character)) return;

	Character->GetMesh()->UnlinkAnimClassLayers(CurrentAnimLayerClass);
	CurrentAnimLayerClass = nullptr;
}
