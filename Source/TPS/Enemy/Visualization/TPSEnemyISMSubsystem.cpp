#include "TPSEnemyISMSubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Enemy/Settings/TPSEnemySettings.h"

void UTPSEnemyISMSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UWorld* pWorld = GetWorld();
	if (!ensure(pWorld)) return;

	// ① ISM을 보유할 임시 Actor 생성 (PIE 중복 방지 — 유니크 이름)
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = MakeUniqueObjectName(pWorld->GetCurrentLevel(), AActor::StaticClass(), TEXT("EnemyISMOwner"));
	ISMOwnerActor = pWorld->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
	if (!ensure(ISMOwnerActor)) return;

	// ② ISM 컴포넌트 생성 + 등록
	ISMComponent = NewObject<UInstancedStaticMeshComponent>(ISMOwnerActor, TEXT("EnemyISM"));
	if (ensure(ISMComponent))
	{
		ISMComponent->RegisterComponent();
		ISMComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ISMComponent->SetCastShadow(false);

		// DeveloperSettings에서 ISM 메시 로드
		const UTPSEnemySettings* Settings = GetDefault<UTPSEnemySettings>();
		if (ensure(Settings && !Settings->EnemyISMMesh.IsNull()))
		{
			UStaticMesh* Mesh = Settings->EnemyISMMesh.LoadSynchronous();
			if (ensure(Mesh))
			{
				ISMComponent->SetStaticMesh(Mesh);
			}
		}
	}
}

void UTPSEnemyISMSubsystem::Deinitialize()
{
	if (ISMOwnerActor)
	{
		ISMOwnerActor->Destroy();
		ISMOwnerActor = nullptr;
	}
	ISMComponent = nullptr;
	FreeIndices.Empty();

	Super::Deinitialize();
}

int32 UTPSEnemyISMSubsystem::AddInstance(const FTransform& InTransform)
{
	if (!ensure(ISMComponent)) return INDEX_NONE;

	// 재활용 가능한 인덱스가 있으면 사용
	if (FreeIndices.Num() > 0)
	{
		const int32 ReuseIndex = FreeIndices.Pop();
		ISMComponent->UpdateInstanceTransform(ReuseIndex, InTransform, true, true);
		return ReuseIndex;
	}

	// 새 인스턴스 추가
	return ISMComponent->AddInstance(InTransform, true);
}

void UTPSEnemyISMSubsystem::RemoveInstance(int32 InInstanceIndex)
{
	if (!ensure(ISMComponent)) return;
	if (InInstanceIndex == INDEX_NONE) return;

	// 실제 제거 대신 화면 밖으로 이동 + 재활용 풀에 등록
	static constexpr float HiddenInstanceZ = -100000.f;
	const FTransform HiddenTransform(FRotator::ZeroRotator, FVector(0.f, 0.f, HiddenInstanceZ));
	ISMComponent->UpdateInstanceTransform(InInstanceIndex, HiddenTransform, true, true);
	FreeIndices.Add(InInstanceIndex);
}

void UTPSEnemyISMSubsystem::UpdateInstanceTransform(int32 InInstanceIndex, const FTransform& InTransform)
{
	if (!ensure(ISMComponent)) return;
	if (InInstanceIndex == INDEX_NONE) return;

	ISMComponent->UpdateInstanceTransform(InInstanceIndex, InTransform, true, true);
}
