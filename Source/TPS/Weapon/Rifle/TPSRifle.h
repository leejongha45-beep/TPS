#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/Interface/Action/Attachable.h"
#include "TPSRifle.generated.h"

UCLASS()
class TPS_API ATPSRifle : public AActor, public IAttachable
{
	GENERATED_BODY()

public:
	ATPSRifle();

	virtual void Attach(USkeletalMeshComponent* InTargetMesh) override;
	virtual void Detach(USkeletalMeshComponent* InTargetMesh) override;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Mesh")
	TObjectPtr<class USkeletalMeshComponent> WeaponMeshInst;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName EquipSocketName = TEXT("weapon_r");

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName UnequipSocketName = TEXT("spine_01");
};