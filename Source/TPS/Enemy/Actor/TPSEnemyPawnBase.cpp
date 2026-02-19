#include "Enemy/Actor/TPSEnemyPawnBase.h"
#include "Enemy/Component/TPSEnemyHealthComponent.h"
#include "Enemy/Mass/Fragment/TPSEnemyHealthFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyAIStateFragment.h"
#include "MassEntitySubsystem.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

DECLARE_LOG_CATEGORY_EXTERN(EnemyAILog, Log, All);
DEFINE_LOG_CATEGORY(EnemyAILog);

namespace TPSEnemyConstants
{
	static const FVector PoolParkingLocation(-10000.f, 0.f, 0.f);
}

ATPSEnemyPawnBase::ATPSEnemyPawnBase()
{
	// ① Capsule (Root) — 충돌/피격 판정
	if (!CapsuleComponentInst)
	{
		CapsuleComponentInst = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
		if (ensure(CapsuleComponentInst))
		{
			SetRootComponent(CapsuleComponentInst);
			CapsuleComponentInst->InitCapsuleSize(22.f, 44.f);
			CapsuleComponentInst->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// ② SkeletalMesh — Capsule에 부착, 기본 숨김 (풀 비활성 상태)
			if (!MeshComponentInst)
			{
				MeshComponentInst = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
				if (ensure(MeshComponentInst))
				{
					MeshComponentInst->SetupAttachment(CapsuleComponentInst);
					MeshComponentInst->SetVisibility(false);
				}
			}
		}
	}

	// ③ HealthComponent
	if (!HealthComponentInst)
	{
		HealthComponentInst = CreateDefaultSubobject<UTPSEnemyHealthComponent>(TEXT("HealthComponent"));
		if (ensure(HealthComponentInst))
		{
		}
	}

	// 풀 비활성 초기 상태
	AActor::SetActorTickEnabled(false);
}

void ATPSEnemyPawnBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// HealthComponent 사망 델리게이트 바인딩
	if (ensure(HealthComponentInst))
	{
		HealthComponentInst->OnEnemyDeathDelegate.AddUObject(this, &ATPSEnemyPawnBase::OnDeath);
	}
}

void ATPSEnemyPawnBase::SyncAIState(EEnemyAIState InNewState)
{
	if (SyncedAIState == InNewState) return;

	const EEnemyAIState PrevState = SyncedAIState;
	SyncedAIState = InNewState;

	// 상태 전환 시 시각/사운드 반응 (추후 확장 지점)
	// 예: Attack 진입 → 공격 애니메이션, Chase 진입 → 달리기 애니메이션
	UE_LOG(EnemyAILog, Verbose, TEXT("[SyncAIState] %s -> %s"),
		*UEnum::GetValueAsString(PrevState),
		*UEnum::GetValueAsString(InNewState));
}

void ATPSEnemyPawnBase::OnDeath()
{
	// ① AI 상태 반영
	SyncedAIState = EEnemyAIState::Die;

	// ② 사망 연출
	PlayDeath();
}

void ATPSEnemyPawnBase::ActivateEnemy(const FTransform& InTransform, EEnemyAIState InInitialState)
{
	// ① 위치/회전 설정
	SetActorLocationAndRotation(InTransform.GetLocation(), InTransform.GetRotation().Rotator());

	// ② 충돌 ON
	if (ensure(CapsuleComponentInst))
	{
		CapsuleComponentInst->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	// ③ 메시 표시
	if (ensure(MeshComponentInst))
	{
		MeshComponentInst->SetVisibility(true);
	}

	// ④ AI 상태 초기화
	SyncedAIState = InInitialState;

	// ⑤ HealthComponent 초기화
	if (ensure(HealthComponentInst))
	{
		HealthComponentInst->InitFromFragment(50.f, 50.f);
	}
}

void ATPSEnemyPawnBase::DeactivateEnemy()
{
	// ① 충돌 OFF
	if (ensure(CapsuleComponentInst))
	{
		CapsuleComponentInst->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// ② 메시 숨김
	if (ensure(MeshComponentInst))
	{
		MeshComponentInst->SetVisibility(false);
	}

	// ③ 맵 밖으로 이동 (풀 대기 위치)
	SetActorLocation(TPSEnemyConstants::PoolParkingLocation);

	// ④ Death 타이머 정리
	GetWorldTimerManager().ClearTimer(DeathTimerHandle);

	// ⑤ Mass Entity Handle 초기화
	MassEntityHandle = FMassEntityHandle();
	CachedEntityManager = nullptr;
}

void ATPSEnemyPawnBase::PlayDeath()
{
	// ① 충돌 즉시 OFF (죽은 적에게 투사체 낭비 방지)
	if (ensure(CapsuleComponentInst))
	{
		CapsuleComponentInst->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// ② 사망 애니메이션 (서브클래스에서 override 예정)

	// ③ 일정 시간 후 비활성화 + 풀 반환
	GetWorldTimerManager().SetTimer(DeathTimerHandle, this,
		&ATPSEnemyPawnBase::DeactivateEnemy, DeathLingerTime, false);
}

float ATPSEnemyPawnBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent,
		EventInstigator, DamageCauser);

	// ① HealthComponent에 데미지 전달
	if (ensure(HealthComponentInst))
	{
		HealthComponentInst->ApplyDamage(ActualDamage);
	}

	// ② Mass Fragment 동기화
	if (MassEntityHandle.IsValid() && CachedEntityManager)
	{
		FTPSEnemyHealthFragment* HealthFrag = CachedEntityManager->GetFragmentDataPtr<FTPSEnemyHealthFragment>(MassEntityHandle);
		if (ensure(HealthFrag))
		{
			HealthFrag->CurrentHealth = HealthComponentInst->GetCurrentHealth();

			// 사망 시 AI 상태도 동기화
			if (HealthComponentInst->IsDead())
			{
				FTPSEnemyAIStateFragment* AIFrag = CachedEntityManager->GetFragmentDataPtr<FTPSEnemyAIStateFragment>(MassEntityHandle);
				if (ensure(AIFrag))
				{
					AIFrag->AIState = EEnemyAIState::Die;
				}
			}
		}
	}

	return ActualDamage;
}

void ATPSEnemyPawnBase::SetMassEntityHandle(FMassEntityHandle InHandle, FMassEntityManager* InEntityManager)
{
	MassEntityHandle = InHandle;
	CachedEntityManager = InEntityManager;
}
