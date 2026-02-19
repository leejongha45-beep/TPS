#include "Enemy/Actor/TPSEnemyPawnBase.h"
#include "Enemy/Component/TPSEnemyHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

DECLARE_LOG_CATEGORY_EXTERN(EnemyAILog, Log, All);
DEFINE_LOG_CATEGORY(EnemyAILog);

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

	// ④ AI Tick 초기 설정
	AIControlTickFunction.bCanEverTick = true;
	AIControlTickFunction.bStartWithTickEnabled = false;
	AIControlTickFunction.TickGroup = TG_PrePhysics;

	// 풀 비활성 초기 상태
	AActor::SetActorTickEnabled(false);
}

void ATPSEnemyPawnBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// ① HealthComponent 사망 델리게이트 바인딩
	if (ensure(HealthComponentInst))
	{
		HealthComponentInst->OnEnemyDeathDelegate.AddUObject(this, &ATPSEnemyPawnBase::OnDeath);
	}
}

void ATPSEnemyPawnBase::RegisterActorTickFunctions(bool bRegister)
{
	Super::RegisterActorTickFunctions(bRegister);

	if (bRegister)
	{
		AIControlTickFunction.Target = this;
		AIControlTickFunction.RegisterTickFunction(GetLevel());
		AIControlTickFunction.SetTickFunctionEnable(AIControlTickFunction.bStartWithTickEnabled);
	}
	else
	{
		if (AIControlTickFunction.IsTickFunctionRegistered())
		{
			AIControlTickFunction.UnRegisterTickFunction();
		}
	}
}

void ATPSEnemyPawnBase::SetAIControlTickEnabled(bool bEnabled)
{
	if (AIControlTickFunction.IsTickFunctionEnabled() == bEnabled) return;

	AIControlTickFunction.SetTickFunctionEnable(bEnabled);
	UE_LOG(EnemyAILog, Warning, TEXT("[SetAIControlTickEnabled] %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void ATPSEnemyPawnBase::AI_Control_Tick(float DeltaTime)
{
	// ① 타겟 참조 (추후: 가장 가까운 아군/플레이어 중 타겟 선정 로직으로 교체)
	APawn* pTarget = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!ensure(pTarget)) return;

	// ② 거리 계산
	const float Distance = FVector::Dist(GetActorLocation(), pTarget->GetActorLocation());

	// ③ 상태별 처리
	switch (CurrentAIState)
	{
	case EEnemyAIState::Idle:   ProcessIdle(DeltaTime);                        break;
	case EEnemyAIState::Chase:  ProcessChase(DeltaTime, pTarget, Distance);    break;
	case EEnemyAIState::Attack: ProcessAttack(DeltaTime, pTarget, Distance);   break;
	case EEnemyAIState::Die:    break;
	}
}

void ATPSEnemyPawnBase::ProcessIdle(float DeltaTime)
{
	// 스폰 후 대기 → Chase
	StateTimer += DeltaTime;
	if (StateTimer >= IdleWaitTime)
	{
		CurrentAIState = EEnemyAIState::Chase;
	}
}

void ATPSEnemyPawnBase::ProcessChase(float DeltaTime, APawn* InTarget, float InDistance)
{
	// ① 타겟 방향으로 이동
	const FVector Direction = (InTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	SetActorLocation(GetActorLocation() + Direction * MoveSpeed * DeltaTime);
	SetActorRotation(Direction.Rotation());

	// ② 사거리 진입 → Attack
	if (InDistance <= AttackRange)
	{
		CurrentAIState = EEnemyAIState::Attack;
	}
}

void ATPSEnemyPawnBase::ProcessAttack(float DeltaTime, APawn* InTarget, float InDistance)
{
	// ① 사거리 이탈 → Chase 복귀 (히스테리시스 1.2배)
	if (InDistance > AttackRange * 1.2f)
	{
		CurrentAIState = EEnemyAIState::Chase;
		return;
	}

	// ② 쿨다운 소진 → 공격
	AttackCooldownTimer -= DeltaTime;
	if (AttackCooldownTimer <= 0.f)
	{
		UGameplayStatics::ApplyDamage(InTarget, AttackDamage, nullptr, this, nullptr);
		AttackCooldownTimer = AttackInterval;
	}
}

void ATPSEnemyPawnBase::OnDeath()
{
	// ① AI 상태 전환
	CurrentAIState = EEnemyAIState::Die;

	// ② AI Tick 정지
	SetAIControlTickEnabled(false);

	// ③ 사망 연출
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
	CurrentAIState = InInitialState;
	StateTimer = 0.f;
	AttackCooldownTimer = 0.f;

	// ⑤ HealthComponent 초기화
	if (ensure(HealthComponentInst))
	{
		HealthComponentInst->InitFromFragment(50.f, 50.f);
	}

	// ⑥ AI Tick 시작
	SetAIControlTickEnabled(true);
}

void ATPSEnemyPawnBase::DeactivateEnemy()
{
	// ① AI Tick 정지
	SetAIControlTickEnabled(false);

	// ② 충돌 OFF
	if (ensure(CapsuleComponentInst))
	{
		CapsuleComponentInst->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// ③ 메시 숨김
	if (ensure(MeshComponentInst))
	{
		MeshComponentInst->SetVisibility(false);
	}

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