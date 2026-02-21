#include "Character/Component/Data/TPSFootstepComponent.h"
#include "Character/Component/Data/TPSPlayerStateComponent.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Sound/SoundBase.h"
#include "Utils/Struct/FootstepData.h"

DEFINE_LOG_CATEGORY(FootstepLog);

UTPSFootstepComponent::UTPSFootstepComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	for (auto& pRow : CachedRows)
	{
		pRow = nullptr;
	}
}

void UTPSFootstepComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheFootstepRows();
}

void UTPSFootstepComponent::CacheFootstepRows()
{
	if (!ensure(FootstepDataTableAsset)) return;

	// ① 전체 Row를 한 번에 가져옴
	TArray<FFootstepSoundRow*> AllRows;
	FootstepDataTableAsset->GetAllRows<FFootstepSoundRow>(TEXT("FootstepCache"), AllRows);

	// ② SurfaceType을 인덱스로 변환하여 캐싱
	for (const FFootstepSoundRow* pRow : AllRows)
	{
		if (!pRow) continue;

		const uint8 Index = static_cast<uint8>(pRow->SurfaceType);
		if (Index < static_cast<uint8>(ESurfaceType::MAX))
		{
			CachedRows[Index] = pRow;
		}
	}

	UE_LOG(FootstepLog, Log, TEXT("FootstepComponent: Cached %d rows from DataTable"), AllRows.Num());
}

const FFootstepSoundRow* UTPSFootstepComponent::FindCachedRow(ESurfaceType SurfaceType) const
{
	const uint8 Index = static_cast<uint8>(SurfaceType);
	if (Index < static_cast<uint8>(ESurfaceType::MAX) && CachedRows[Index])
	{
		return CachedRows[Index];
	}

	// fallback: Default Row
	const uint8 DefaultIndex = static_cast<uint8>(ESurfaceType::Default);
	if (ensure(CachedRows[DefaultIndex]))
	{
		UE_LOG(FootstepLog, Warning,
		       TEXT("FootstepComponent: Row for SurfaceType [%d] not cached, falling back to Default"),
		       Index);
		return CachedRows[DefaultIndex];
	}

	return nullptr;
}

void UTPSFootstepComponent::PlayFootstepSound(
	USkeletalMeshComponent* MeshComp,
	const FName& FootBoneName,
	float TraceDistance)
{
	if (!ensure(MeshComp)) return;

	AActor* pOwner = MeshComp->GetOwner();
	if (!ensure(pOwner)) return;

	UWorld* pWorld = pOwner->GetWorld();
	if (!ensure(pWorld)) return;

	// ① LineTrace — 발 본 위치에서 아래로
	FVector TraceStart = MeshComp->GetSocketLocation(FootBoneName);
	FVector ImpactPoint = TraceStart;

	ESurfaceType SurfaceType = DetectSurfaceType(
		pWorld, TraceStart, TraceDistance, pOwner, ImpactPoint);

	// ② 캐시에서 Row 조회 (O(1))
	const FFootstepSoundRow* pRow = FindCachedRow(SurfaceType);
	if (!ensure(pRow)) return;

	// ③ Walk / Run 분기
	UTPSPlayerStateComponent* pStateComp =
		pOwner->FindComponentByClass<UTPSPlayerStateComponent>();

	uint8 bIsSprinting = 0;
	if (ensure(pStateComp))
	{
		bIsSprinting = pStateComp->HasState(EActionState::Sprinting) ? 1 : 0;
	}

	const TArray<TObjectPtr<USoundBase>>& SoundArray =
		(bIsSprinting && pRow->RunSoundAssets.Num() > 0)
			? pRow->RunSoundAssets
			: pRow->WalkSoundAssets;

	if (SoundArray.Num() == 0)
	{
		UE_LOG(FootstepLog, Warning,
		       TEXT("FootstepComponent: No sound assets for SurfaceType [%d]"),
		       static_cast<uint8>(SurfaceType));
		return;
	}

	// ④ 랜덤 사운드 선택 + 재생
	int32 Index = FMath::RandRange(0, SoundArray.Num() - 1);
	USoundBase* pSound = SoundArray[Index].Get();
	if (!ensure(pSound)) return;

	float FinalPitch = 1.f + FMath::FRandRange(-pRow->PitchVariation, pRow->PitchVariation);

	UGameplayStatics::PlaySoundAtLocation(
		pWorld,
		pSound,
		ImpactPoint,
		pRow->VolumeMultiplier,
		FinalPitch
		);
}

ESurfaceType UTPSFootstepComponent::DetectSurfaceType(
	UWorld* World,
	const FVector& TraceStart,
	float TraceDistance,
	AActor* IgnoreActor,
	FVector& OutImpactPoint)
{
	if (!ensure(World)) return ESurfaceType::Default;

	FVector TraceEnd = TraceStart - FVector(0.f, 0.f, TraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(IgnoreActor);
	Params.bReturnPhysicalMaterial = true;

	if (!World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params))
	{
		return ESurfaceType::Default;
	}

	OutImpactPoint = HitResult.ImpactPoint;

	UPhysicalMaterial* pPhysMat = HitResult.PhysMaterial.Get();
	if (!pPhysMat)
	{
		return ESurfaceType::Default;
	}

	return ConvertPhysicalSurface(UPhysicalMaterial::DetermineSurfaceType(pPhysMat));
}

ESurfaceType UTPSFootstepComponent::ConvertPhysicalSurface(EPhysicalSurface InSurface)
{
	switch (InSurface)
	{
	case SurfaceType1: return ESurfaceType::Concrete;
	case SurfaceType2: return ESurfaceType::Dirt;
	case SurfaceType3: return ESurfaceType::Grass;
	case SurfaceType4: return ESurfaceType::Snow;
	case SurfaceType5: return ESurfaceType::Metal;
	case SurfaceType6: return ESurfaceType::Water;
	case SurfaceType7: return ESurfaceType::Wood;
	case SurfaceType8: return ESurfaceType::Mud;
	default: return ESurfaceType::Default;
	}
}