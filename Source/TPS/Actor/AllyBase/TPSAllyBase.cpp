#include "Actor/AllyBase/TPSAllyBase.h"
#include "Components/StaticMeshComponent.h"
#include "Core/Subsystem/TPSTargetSubsystem.h"
#include "Core/Subsystem/TPSDamageSubsystem.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTPSBase, Log, All);
DEFINE_LOG_CATEGORY(LogTPSBase);

ATPSAllyBase::ATPSAllyBase()
{
	// ① SceneRoot
	if (!SceneRootInst)
	{
		SceneRootInst = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
		if (ensure(SceneRootInst.Get()))
		{
			RootComponent = SceneRootInst.Get();

			// ② StaticMesh — SceneRoot에 부착
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

void ATPSAllyBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	// ① Subsystem에 ITargetable 등록 + 기지 위치 설정
	if (UTPSTargetSubsystem* TargetSS = GetWorld()->GetSubsystem<UTPSTargetSubsystem>())
	{
		TargetSS->RegisterTargetableActor(TScriptInterface<ITargetable>(this));
		TargetSS->SetAllyBaseLocation(GetActorLocation());
	}

	// ② IDamageable 등록
	if (UTPSDamageSubsystem* DamageSS = GetWorld()->GetSubsystem<UTPSDamageSubsystem>())
	{
		DamageSS->RegisterDamageableActor(TScriptInterface<IDamageable>(this));
	}

	UE_LOG(LogTPSBase, Log, TEXT("[AllyBase] %s initialized — HP: %.0f / %.0f"),
		*GetName(), CurrentHealth, MaxHealth);
}

float ATPSAllyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
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

float ATPSAllyBase::ReceiveDamage(float Damage, AActor* DamageCauser)
{
	if (bIsDestroyed) return 0.f;

	// UE 데미지 파이프라인 활용
	FDamageEvent DamageEvent;
	return TakeDamage(Damage, DamageEvent, nullptr, DamageCauser);
}

void ATPSAllyBase::OnBaseDestroyed()
{
	if (bIsDestroyed) return;
	bIsDestroyed = true;

	// ① Subsystem에서 해제
	if (UTPSTargetSubsystem* TargetSS = GetWorld()->GetSubsystem<UTPSTargetSubsystem>())
	{
		TargetSS->UnregisterTargetableActor(TScriptInterface<ITargetable>(this));
	}
	if (UTPSDamageSubsystem* DamageSS = GetWorld()->GetSubsystem<UTPSDamageSubsystem>())
	{
		DamageSS->UnregisterDamageableActor(TScriptInterface<IDamageable>(this));
	}

	UE_LOG(LogTPSBase, Warning, TEXT("[AllyBase] %s destroyed!"), *GetName());

	OnBaseDestroyedDelegate.Broadcast();

	// 향후: 파괴 연출 (폭발 이펙트, 사운드, 메시 교체 등)
}