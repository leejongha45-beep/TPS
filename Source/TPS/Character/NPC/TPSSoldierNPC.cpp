#include "Character/NPC/TPSSoldierNPC.h"
#include "Character/Component/Data/TPSAnimLayerComponent.h"
#include "Character/Component/Action/TPSFireComponent.h"
#include "Character/Component/Data/TPSPlayerStateComponent.h"
#include "Core/Controller/TPSNPCController.h"
#include "Core/Subsystem/TPSTargetSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Weapon/TPSWeaponBase.h"

ATPSSoldierNPC::ATPSSoldierNPC(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AIControllerClass = ATPSNPCController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw = true;
}

void ATPSSoldierNPC::BeginPlay()
{
	Super::BeginPlay();

	// CMC: 컨트롤러 회전 사용 → OrientRotationToMovement 끄기
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		CMC->bOrientRotationToMovement = false;
	}

	// 애니메이션 최적화 — 거리 기반 업데이트 빈도 감소 + 분산
	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		SkelMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

		// NPC마다 틱 인터벌을 분산 → 한 프레임에 몰리지 않게 (초당 5회)
		const float BaseInterval = 0.2f;
		const float Jitter = FMath::FRandRange(0.f, 0.1f);
		SkelMesh->SetComponentTickInterval(BaseInterval + Jitter);
	}

	// ALI 링크 — RifleHipFire 레이어
	if (auto* pAnimLayerComp = FindComponentByClass<UTPSAnimLayerComponent>())
	{
		pAnimLayerComp->LinkAnimLayer(pAnimLayerComp->GetRifleHipFireLayerClass());
	}

	// 무기 스폰 + 소켓 부착
	if (WeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		SpawnedWeapon = GetWorld()->SpawnActor<ATPSWeaponBase>(WeaponClass, SpawnParams);
		if (SpawnedWeapon)
		{
			SpawnedWeapon->Attach(GetMesh());
		}
	}

	if (UTPSTargetSubsystem* TargetSS = GetWorld()->GetSubsystem<UTPSTargetSubsystem>())
	{
		TargetSS->RegisterNPC(this);
	}

	// 풀 시스템용 — 모든 초기화 후 CMC 틱 해제 (ActivateNPC에서 재등록)
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		CMC->SetComponentTickEnabled(false);
		if (CMC->PrimaryComponentTick.IsTickFunctionRegistered())
		{
			CMC->PrimaryComponentTick.UnRegisterTickFunction();
		}
	}
}

void ATPSSoldierNPC::StartFire()
{
	if (!SpawnedWeapon || !FireComponentInst.Get()) { return; }
	if (FireComponentInst->GetIsFiring()) { return; }

	// NPC ViewPoint = 눈 위치 + 컨트롤러 회전
	auto ViewPointGetter = [this](FVector& OutLocation, FRotator& OutRotation)
	{
		OutLocation = GetPawnViewLocation();
		OutRotation = GetControlRotation();
	};

	FireComponentInst->StartFire(SpawnedWeapon.Get(), MoveTemp(ViewPointGetter));
}

void ATPSSoldierNPC::Reload()
{
	if (!SpawnedWeapon || !FireComponentInst.Get()) { return; }
	if (FireComponentInst->GetIsReloading()) { return; }
	if (SpawnedWeapon->HasAmmo()) { return; }

	// 사격 중이면 먼저 중단
	if (FireComponentInst->GetIsFiring())
	{
		StopFire();
	}

	FireComponentInst->StartReload(SpawnedWeapon.Get());
}

void ATPSSoldierNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UTPSTargetSubsystem* TargetSS = GetWorld()->GetSubsystem<UTPSTargetSubsystem>())
	{
		TargetSS->UnregisterNPC(this);
	}

	Super::EndPlay(EndPlayReason);
}