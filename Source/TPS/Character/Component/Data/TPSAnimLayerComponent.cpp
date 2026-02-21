#include "Character/Component/Data/TPSAnimLayerComponent.h"
#include "GameFramework/Character.h"
#include "Animation/Player/TPSLinkedAnimInstance.h"

UTPSAnimLayerComponent::UTPSAnimLayerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTPSAnimLayerComponent::LinkAnimLayer(TSubclassOf<UTPSLinkedAnimInstance> InClass)
{
	if (!InClass || InClass == CurrentAnimLayerClass) return;

	ACharacter* pCharacter = Cast<ACharacter>(GetOwner());
	if (!ensure(pCharacter)) return;

	USkeletalMeshComponent* pMeshComp = pCharacter->GetMesh();
	if (ensure(pMeshComp))
	{
		pMeshComp->LinkAnimClassLayers(InClass);
		CurrentAnimLayerClass = InClass;
	}
}

void UTPSAnimLayerComponent::UnlinkAnimLayer()
{
	if (!CurrentAnimLayerClass) return;

	ACharacter* pCharacter = Cast<ACharacter>(GetOwner());
	if (!ensure(pCharacter)) return;

	USkeletalMeshComponent* pMeshComp = pCharacter->GetMesh();
	if (ensure(pMeshComp))
	{
		pMeshComp->UnlinkAnimClassLayers(CurrentAnimLayerClass);
		CurrentAnimLayerClass = nullptr;
	}
}