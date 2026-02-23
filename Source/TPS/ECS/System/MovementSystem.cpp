#include "ECS/System/MovementSystem.h"
#include "ECS/Component/Components.h"

/** ② Write: 캐싱된 읽기값을 파라미터로 받아 Current에만 쓰기 */
void Write(CTransform& OutTransform, float DeltaTime, const FVector& CachedVelocity)
{
	OutTransform.Position += CachedVelocity * DeltaTime;
}

/** ③ PushToPrev: 갱신된 Current → Prev 복사 (다음 프레임용) */
void PushToPrev(CTransformPrev& OutPrev, const CTransform& InCurrent)
{
	OutPrev.Position = InCurrent.Position;
	OutPrev.Rotation = InCurrent.Rotation;
}

/**
 * Read는 Cached 지역변수로 값을 복사해야 하므로 함수 래핑 불가 — Tick 본문에 직접 노출
 * Write/PushToPrev만 함수로 분리하여 Tick 루프의 가독성 확보
 */
void MovementSystem::Tick(entt::registry& Registry, float DeltaTime)
{
	auto View = Registry.view<CTransform, CTransformPrev, CMovementPrev, CEnemyStatePrev>();
	for (auto Entity : View)
	{
		// ① Read
		const EEnemyState CachedState = View.get<CEnemyStatePrev>(Entity).State;
		if (CachedState != EEnemyState::Moving) { continue; }

		const FVector CachedVelocity = View.get<CMovementPrev>(Entity).Velocity;

		// ② Write
		Write(View.get<CTransform>(Entity), DeltaTime, CachedVelocity);

		// ③ PushToPrev
		PushToPrev(View.get<CTransformPrev>(Entity), View.get<CTransform>(Entity));
	}
}
