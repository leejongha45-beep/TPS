#include "Actor/Base/TPSDestroyableBase.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTPSBase, Log, All);
DEFINE_LOG_CATEGORY(LogTPSBase);

ATPSDestroyableBase::ATPSDestroyableBase()
{
	if (!SceneRootInst)
	{
		SceneRootInst = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
		if (ensure(SceneRootInst.Get()))
		{
			RootComponent = SceneRootInst.Get();

			if (!MeshComponentInst)
			{
				MeshComponentInst = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
				if (ensure(MeshComponentInst.Get()))
				{
					MeshComponentInst->SetupAttachment(SceneRootInst);
				}
			}
		}
	}
}

void ATPSDestroyableBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

float ATPSDestroyableBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDestroyed) return 0.f;

	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent,
		EventInstigator, DamageCauser);

	CurrentHealth = FMath::Max(0.f, CurrentHealth - ActualDamage);

	if (CurrentHealth <= 0.f)
	{
		OnBaseDestroyed();
	}

	return ActualDamage;
}

float ATPSDestroyableBase::ReceiveDamage(float Damage, AActor* DamageCauser)
{
	if (bIsDestroyed) return 0.f;

	FDamageEvent DamageEvent;
	return TakeDamage(Damage, DamageEvent, nullptr, DamageCauser);
}

void ATPSDestroyableBase::OnBaseDestroyed()
{
	if (bIsDestroyed) return;
	bIsDestroyed = true;

	UE_LOG(LogTPSBase, Warning, TEXT("[DestroyableBase] %s destroyed!"), *GetName());

	OnBaseDestroyedDelegate.Broadcast();
}
