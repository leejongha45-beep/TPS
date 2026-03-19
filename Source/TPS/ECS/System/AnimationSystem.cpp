#include "ECS/System/AnimationSystem.h"
#include "ECS/Component/Components.h"
#include "Async/ParallelFor.h"


void AnimationSystem::Tick(entt::registry& Registry, float DeltaTime)
{
	auto View = Registry.view<CAnimation, CAnimationPrev, CEnemyStatePrev, CLODPrev>();

	// ── Entity 수집 ──
	TArray<entt::entity, TInlineAllocator<3000>> Entities;
	Entities.Reserve(View.size_hint());
	for (auto Entity : View) { Entities.Add(Entity); }

	const int32 Count = Entities.Num();

	// ── ParallelFor: Entity별 독립 처리 ── [WorkerThread]
	ParallelFor(Count, [&View, &Entities](int32 Index)
	{
		const entt::entity Entity = Entities[Index];

		// ① Read — Prev → 지역변수
		const bool bCachedShouldTick = View.get<CLODPrev>(Entity).bShouldTick;
		if (!bCachedShouldTick) { return; }

		const float CachedAccumDT   = View.get<CLODPrev>(Entity).AccumulatedDeltaTime;
		const float CachedAnimIndex = View.get<CAnimationPrev>(Entity).AnimIndex;
		const float CachedAnimTime  = View.get<CAnimationPrev>(Entity).AnimTime;
		const float CachedPlayRate  = View.get<CAnimationPrev>(Entity).PlayRate;
		const EEnemyState CachedState = View.get<CEnemyStatePrev>(Entity).State;

		// ② 계산 — 지역변수만 사용
		float NewAnimIndex;
		switch (CachedState)
		{
		case EEnemyState::Idle:           NewAnimIndex = 0.f; break;
		case EEnemyState::AttackCooldown: NewAnimIndex = 0.f; break;
		case EEnemyState::Moving:         NewAnimIndex = 1.f; break;
		case EEnemyState::AttackReady:    NewAnimIndex = 2.f; break;
		case EEnemyState::Attacking:      NewAnimIndex = 3.f; break;
		case EEnemyState::Dying:          NewAnimIndex = 4.f; break;
		case EEnemyState::Dead:           NewAnimIndex = 4.f; break;
		default:                          NewAnimIndex = 0.f; break;
		}

		float NewAnimTime;
		if (NewAnimIndex != CachedAnimIndex)
		{
			NewAnimTime = 0.f;
		}
		else if (CachedState == EEnemyState::Dying)
		{
			NewAnimTime = FMath::Min(CachedAnimTime + (CachedAccumDT * CachedPlayRate), ECSConstants::DeathAnimDuration);
		}
		else if (CachedState == EEnemyState::Dead)
		{
			NewAnimTime = ECSConstants::DeathAnimDuration;
		}
		else
		{
			NewAnimTime = CachedAnimTime + (CachedAccumDT * CachedPlayRate);
		}

		// ③ Write — Current에 쓰기
		auto& Anim = View.get<CAnimation>(Entity);
		Anim.AnimIndex = NewAnimIndex;
		Anim.AnimTime = NewAnimTime;

		// ④ PushToPrev
		auto& AnimPrev = View.get<CAnimationPrev>(Entity);
		AnimPrev.AnimIndex = NewAnimIndex;
		AnimPrev.AnimTime = NewAnimTime;
		AnimPrev.PlayRate = Anim.PlayRate;
	});
}
