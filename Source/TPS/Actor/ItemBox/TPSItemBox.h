#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/Interface/Action/Interactable.h"
#include "Utils/UENUM/WeaponType.h"
#include "TPSItemBox.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogItemBox, Log, All);

UCLASS()
class TPS_API ATPSItemBox : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	ATPSItemBox();

	FORCEINLINE const TScriptInterface<class IInteractable>& GetInteractableInterface() const { return InteractableInterface; }

	void SpawnWeapon(EWeaponType TargetWeapon);

protected:
	virtual void Interact() override;

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                       bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Collision")
	TObjectPtr<class UBoxComponent> BoxCollisionInst;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Mesh")
	TObjectPtr<class UStaticMeshComponent> MeshInst;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Interaction")
	TObjectPtr<class UTPSItemBoxInteractionComponent> InteractionComponentInst;

	UPROPERTY()
	TScriptInterface<class IInteractable> InteractableInterface;

	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	TArray<TSubclassOf<class ATPSWeaponBase>> ItemClassArray;

	UPROPERTY(VisibleDefaultsOnly, Category="Weapon")
	TObjectPtr<class ATPSWeaponBase> SpawnedWeapon;
};