#include "TPSMeleeEnemyTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "Enemy/Mass/Fragment/TPSEnemyHealthFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyAIStateFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyMovementFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyTypeFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyLODFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyActorRefFragment.h"

void UTPSMeleeEnemyTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	// ① HealthFragment — 체력
	FTPSEnemyHealthFragment& HealthFrag = BuildContext.AddFragment_GetRef<FTPSEnemyHealthFragment>();
	HealthFrag.MaxHealth = DefaultMaxHealth;
	HealthFrag.CurrentHealth = DefaultMaxHealth;

	// ② AIStateFragment — AI 상태머신 데이터
	FTPSEnemyAIStateFragment& AIFrag = BuildContext.AddFragment_GetRef<FTPSEnemyAIStateFragment>();
	AIFrag.AIState = EEnemyAIState::Chase;
	AIFrag.AttackRange = DefaultAttackRange;
	AIFrag.AttackDamage = DefaultAttackDamage;
	AIFrag.AttackInterval = DefaultAttackInterval;

	// ③ MovementFragment — 이동
	FTPSEnemyMovementFragment& MoveFrag = BuildContext.AddFragment_GetRef<FTPSEnemyMovementFragment>();
	MoveFrag.MoveSpeed = DefaultMoveSpeed;

	// ④ TypeFragment — 적 타입
	FTPSEnemyTypeFragment& TypeFrag = BuildContext.AddFragment_GetRef<FTPSEnemyTypeFragment>();
	TypeFrag.EnemyType = EEnemyType::Melee;

	// ⑤ LODFragment — LOD (초기 None)
	BuildContext.AddFragment<FTPSEnemyLODFragment>();

	// ⑥ ActorRefFragment — Actor 참조 (초기 nullptr)
	BuildContext.AddFragment<FTPSEnemyActorRefFragment>();
}
