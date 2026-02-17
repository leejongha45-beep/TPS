#include "Utils/Helpers/FootstepHelper.h"
#include "Utils/Struct/FootstepData.h"
#include "Component/Data/TPSPlayerStateComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"

DECLARE_LOG_CATEGORY_EXTERN(FootstepLog, Log, All);

DEFINE_LOG_CATEGORY(FootstepLog);

void FFootstepHelper::PlayFootstepSound(
	USkeletalMeshComponent* MeshComp,
	const FName& FootBoneName,
	UDataTable* DataTableAsset,
	float TraceDistance)
{
	if (!ensure(MeshComp)) return;
	if (!ensure(DataTableAsset)) return;

	AActor* pOwner = MeshComp->GetOwner();
	if (!ensure(pOwner)) return;

	UWorld* pWorld = pOwner->GetWorld();
	if (!ensure(pWorld)) return;

	// ① LineTrace — 발 본 위치에서 아래로
	FVector TraceStart = MeshComp->GetSocketLocation(FootBoneName);
	FVector ImpactPoint = TraceStart;

	ESurfaceType SurfaceType = DetectSurfaceType(
		pWorld, TraceStart, TraceDistance, pOwner, ImpactPoint);

	// ② DataTable 조회
	const FFootstepSoundRow* pRow = FindFootstepRow(DataTableAsset, SurfaceType);
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
		       TEXT("FootstepHelper: No sound assets for SurfaceType [%s]"),
		       *UEnum::GetValueAsString(SurfaceType));
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

ESurfaceType FFootstepHelper::DetectSurfaceType(
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

ESurfaceType FFootstepHelper::ConvertPhysicalSurface(EPhysicalSurface InSurface)
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

const FFootstepSoundRow* FFootstepHelper::FindFootstepRow(
	const UDataTable* DataTableAsset,
	ESurfaceType SurfaceType)
{
	if (!ensure(DataTableAsset)) return nullptr;

	// ESurfaceType 문자열로 RowName 조회
	FString RowName = UEnum::GetValueAsString(SurfaceType);
	// "ESurfaceType::Concrete" → "Concrete" 로 변환
	RowName.RemoveFromStart(TEXT("ESurfaceType::"));

	const FFootstepSoundRow* pRow =
		DataTableAsset->FindRow<FFootstepSoundRow>(*RowName, TEXT("FootstepLookup"));

	// fallback: Default Row
	if (!pRow)
	{
		UE_LOG(FootstepLog, Warning,
		       TEXT("FootstepHelper: Row [%s] not found, falling back to Default"),
		       *RowName);

		pRow = DataTableAsset->FindRow<FFootstepSoundRow>(
			TEXT("Default"), TEXT("FootstepFallback"));
	}

	return pRow;
}