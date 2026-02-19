#include "Enemy/Actor/TPSEnemyPawnBase.h"
#include "Enemy/Component/TPSEnemyHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

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
			// 추후 델리게이트 바인딩 추가
		}
	}

	// 풀 비활성 초기 상태
	SetActorTickEnabled(false);
}

void ATPSEnemyPawnBase::ActivateEnemy(const FTransform& InTransform)
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

	// ④ Tick 활성화
	SetActorTickEnabled(true);
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

	// ③ Tick 비활성화
	SetActorTickEnabled(false);

	// ④ 맵 밖으로 이동 (풀 대기 위치)
	SetActorLocation(FVector(-10000.f, 0.f, 0.f));

	// ⑤ Death 타이머 정리
	GetWorldTimerManager().ClearTimer(DeathTimerHandle);
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

	// ② Mass Fragment 동기화 — Phase 4에서 구현

	return ActualDamage;
}
