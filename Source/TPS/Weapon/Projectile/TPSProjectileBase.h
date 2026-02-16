#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPSProjectileBase.generated.h"

UCLASS()
class TPS_API ATPSProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	ATPSProjectileBase();

	void ActivateProjectile(const FTransform& InMuzzleTransform, const FVector& InDirection, float InDamage, float InSpeed);
	void DeactivateProjectile();

protected:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit);

	void OnLifeSpanExpired();

	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Collision")
	TObjectPtr<class USphereComponent> CollisionComponentInst;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Movement")
	TObjectPtr<class UProjectileMovementComponent> ProjectileMovementInst;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TObjectPtr<class UNiagaraSystem> ImpactEffectAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float LifeSpan = 3.f;

	float Damage = 0.f;

	FTimerHandle LifeSpanTimerHandle;
};
