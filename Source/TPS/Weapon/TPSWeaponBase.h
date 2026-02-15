#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/Interface/Action/Attachable.h"
#include "TPSWeaponBase.generated.h"

UCLASS()
class TPS_API ATPSWeaponBase : public AActor, public IAttachable
{
	GENERATED_BODY()

public:
	ATPSWeaponBase();

	virtual void Attach(USkeletalMeshComponent* InTargetMesh) override;
	virtual void Detach(USkeletalMeshComponent* InTargetMesh) override;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Mesh")
	TObjectPtr<class USkeletalMeshComponent> WeaponMeshInst;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName EquipSocketName = TEXT("weapon_r_Handle");

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName UnequipSocketName = TEXT("spine_01");
};