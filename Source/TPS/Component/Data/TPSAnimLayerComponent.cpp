#include "Component/Data/TPSAnimLayerComponent.h"
#include "GameFramework/Character.h"
#include "Animation/Player/TPSLinkedAnimInstance.h"

UTPSAnimLayerComponent::UTPSAnimLayerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTPSAnimLayerComponent::LinkAnimLayer(TSubclassOf<UTPSLinkedAnimInstance> InClass)
{
	if (!InClass || InClass == CurrentAnimLayerClass) return;

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!ensure(Character)) return;

	USkeletalMeshComponent* pMeshComp = Character->GetMesh();
	if (ensure(pMeshComp))
	{
		pMeshComp->LinkAnimClassLayers(InClass);
		CurrentAnimLayerClass = InClass;
	}
}

void UTPSAnimLayerComponent::UnlinkAnimLayer()
{
	if (!CurrentAnimLayerClass) return;

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!ensure(Character)) return;

	Character->GetMesh()->UnlinkAnimClassLayers(CurrentAnimLayerClass);
	CurrentAnimLayerClass = nullptr;
}