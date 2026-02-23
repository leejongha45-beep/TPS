#include "ECS/System/AnimationSystem.h"
#include "ECS/Component/Components.h"
#include "Async/ParallelFor.h"

/** ② Write: 상태 기반 AnimIndex 결정 + AnimTime 갱신 (상태 전환 시 리셋) */
void Write(CAnimation& OutAnim, float DeltaTime, float CachedAnimTime,
           float CachedPlayRate, float CachedAnimIndex, EEnemyState CachedState)
{
	float NewAnimIndex;
	switch (CachedState)
	{
	case EEnemyState::Idle:           NewAnimIndex = 0.f; break;
	case EEnemyState::AttackCooldown: NewAnimIndex = 0.f; break;
	case EEnemyState::Moving:         NewAnimIndex = 1.f; break;
	case EEnemyState::AttackReady: NewAnimIndex = 2.f; break;
	case EEnemyState::Attacking:   NewAnimIndex = 3.f; break;
	case EEnemyState::Dying:       NewAnimIndex = 4.f; break;
	case EEnemyState::Dead:        NewAnimIndex = 4.f; break;
	default:                       NewAnimIndex = 0.f; break;
	}

	OutAnim.AnimIndex = NewAnimIndex;

	// 상태 전환 시 AnimTime 리셋 — 이전 애니메이션의 잔류값 방지
	if (NewAnimIndex != CachedAnimIndex)
	{
		OutAnim.AnimTime = 0.f;
		return;
	}

	if (CachedState == EEnemyState::Dying)
	{
		// 비루핑 — Duration에서 클램프
		OutAnim.AnimTime = FMath::Min(CachedAnimTime + (DeltaTime * CachedPlayRate), ECSConstants::DeathAnimDuration);
	}
	else if (CachedState == EEnemyState::Dead)
	{
		// 최종 포즈 유지
		OutAnim.AnimTime = ECSConstants::DeathAnimDuration;
	}
	else
	{
		OutAnim.AnimTime = CachedAnimTime + (DeltaTime * CachedPlayRate);
	}
}

/** ③ PushToPrev: CAnimation → CAnimationPrev */
void PushToPrev(CAnimationPrev& OutPrev, const CAnimation& InCurrent)
{
	OutPrev.AnimIndex = InCurrent.AnimIndex;
	OutPrev.AnimTime  = InCurrent.AnimTime;
	OutPrev.PlayRate  = InCurrent.PlayRate;
}

void AnimationSystem::Tick(entt::registry& Registry, float DeltaTime)
{
	auto View = Registry.view<CAnimation, CAnimationPrev, CEnemyStatePrev>();

	// ── Entity 수집 ──
	TArray<entt::entity, TInlineAllocator<3000>> Entities;
	Entities.Reserve(View.size_hint());
	for (auto Entity : View) { Entities.Add(Entity); }

	const int32 Count = Entities.Num();

	// ── ParallelFor: Entity별 독립 처리 ── [WorkerThread]
	ParallelFor(Count, [&](int32 Index)
	{
		const entt::entity Entity = Entities[Index];

		// ① Read: Prev → Cached 지역변수
		const float CachedAnimIndex   = View.get<CAnimationPrev>(Entity).AnimIndex;
		const float CachedAnimTime    = View.get<CAnimationPrev>(Entity).AnimTime;
		const float CachedPlayRate    = View.get<CAnimationPrev>(Entity).PlayRate;
		const EEnemyState CachedState = View.get<CEnemyStatePrev>(Entity).State;

		// ② Write
		Write(View.get<CAnimation>(Entity), DeltaTime, CachedAnimTime, CachedPlayRate, CachedAnimIndex, CachedState);

		// ③ PushToPrev
		PushToPrev(View.get<CAnimationPrev>(Entity), View.get<CAnimation>(Entity));
	});
}
