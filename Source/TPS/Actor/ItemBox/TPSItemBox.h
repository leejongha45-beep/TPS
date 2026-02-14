#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/Interface/Action/Interactable.h"
#include "TPSItemBox.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogItemBox, Log, All);

UCLASS()
class TPS_API ATPSItemBox : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	ATPSItemBox();

	virtual void Interact() override;

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Collision")
	TObjectPtr<class UBoxComponent> BoxCollisionInst;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Mesh")
	TObjectPtr<class UStaticMeshComponent> MeshInst;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Interaction")
	TObjectPtr<class UTPSItemBoxInteractionComponent> InteractionComponentInst;

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
