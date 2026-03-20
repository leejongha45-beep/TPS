#include "Core/Subsystem/TPSNPCPoolSubsystem.h"
#include "Character/NPC/TPSSoldierNPC.h"
#include "Core/Subsystem/TPSTargetSubsystem.h"
#include "Core/Controller/TPSNPCController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UTPSNPCPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Pool.Reserve(MaxPoolSize);

	// NPCClass가 아직 설정되지 않았으면 소프트 로딩
	if (!NPCClass)
	{
		UClass* Loaded = NPCClassPath.TryLoadClass<ATPSSoldierNPC>();
		if (Loaded)
		{
			NPCClass = Loaded;
			UE_LOG(LogTemp, Log, TEXT("[NPCPool] NPCClass loaded: %s"), *Loaded->GetName());
		}
	}
}

void UTPSNPCPoolSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (!NPCClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[NPCPool] NPCClass not set. Call SetNPCClass() before OnWorldBeginPlay."));
		return;
	}

	// 초기 배치 스폰
	SpawnBatch(InitialSpawnCount);
	UE_LOG(LogTemp, Log, TEXT("[NPCPool] Initial pool: %d / %d"), TotalSpawnedCount, MaxPoolSize);

	// 나머지 지연 스폰
	if (TotalSpawnedCount < MaxPoolSize)
	{
		InWorld.GetTimerManager().SetTimer(
			DeferredSpawnTimerHandle, this, &UTPSNPCPoolSubsystem::DeferredSpawn,
			0.1f, true);
	}
}

void UTPSNPCPoolSubsystem::Deinitialize()
{
	UWorld* pWorld = GetWorld();
	if (pWorld)
	{
		pWorld->GetTimerManager().ClearTimer(DeferredSpawnTimerHandle);
	}

	Pool.Empty();
	TotalSpawnedCount = 0;

	Super::Deinitialize();
}

ATPSSoldierNPC* UTPSNPCPoolSubsystem::GetNPC()
{
	if (Pool.Num() > 0)
	{
		ATPSSoldierNPC* NPC = Pool.Pop();
		return NPC;
	}

	// 풀 고갈 → 긴급 스폰
	UE_LOG(LogTemp, Warning, TEXT("[NPCPool] Pool exhausted! Emergency spawning NPC."));

	UWorld* pWorld = GetWorld();
	if (!pWorld || !NPCClass) { return nullptr; }

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATPSSoldierNPC* NPC = pWorld->SpawnActor<ATPSSoldierNPC>(
		NPCClass, FTransform(FVector(0.f, 0.f, -10000.f)), SpawnParams);
	if (NPC)
	{
		++TotalSpawnedCount;
	}
	return NPC;
}

void UTPSNPCPoolSubsystem::ReturnNPC(ATPSSoldierNPC* InNPC)
{
	if (!InNPC) { return; }

	DeactivateNPC(InNPC);
	Pool.Push(InNPC);
}

void UTPSNPCPoolSubsystem::SpawnBatch(int32 InCount)
{
	UWorld* pWorld = GetWorld();
	if (!pWorld || !NPCClass) { return; }

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < InCount; ++i)
	{
		ATPSSoldierNPC* NPC = pWorld->SpawnActor<ATPSSoldierNPC>(
			NPCClass, FTransform(FVector(0.f, 0.f, -10000.f)), SpawnParams);
		if (NPC)
		{
			DeactivateNPC(NPC);
			Pool.Add(NPC);
			++TotalSpawnedCount;
		}
	}
}

void UTPSNPCPoolSubsystem::DeactivateNPC(ATPSSoldierNPC* InNPC)
{
	if (!InNPC) { return; }

	// TargetSubsystem에서 해제
	if (UTPSTargetSubsystem* TargetSS = GetWorld()->GetSubsystem<UTPSTargetSubsystem>())
	{
		TargetSS->UnregisterNPC(InNPC);
	}

	InNPC->SetActorHiddenInGame(true);
	InNPC->SetActorEnableCollision(false);
	InNPC->SetActorTickEnabled(false);
	InNPC->SetActorLocation(FVector(0.f, 0.f, -10000.f));

	// 스켈레탈 메쉬 틱 비활성화
	if (USkeletalMeshComponent* SkelMesh = InNPC->GetMesh())
	{
		SkelMesh->SetComponentTickEnabled(false);
	}
}

void UTPSNPCPoolSubsystem::ActivateNPC(ATPSSoldierNPC* InNPC, const FVector& InLocation)
{
	if (!InNPC) { return; }

	// Z값을 지형 위로 보정
	FVector SafeLocation = InLocation;
	if (UWorld* pWorld = GetWorld())
	{
		FHitResult Hit;
		const FVector TraceStart = FVector(InLocation.X, InLocation.Y, InLocation.Z + 5000.f);
		const FVector TraceEnd = FVector(InLocation.X, InLocation.Y, InLocation.Z - 5000.f);
		if (pWorld->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
		{
			SafeLocation.Z = Hit.Location.Z + 100.f;
		}
	}

	InNPC->SetActorLocation(SafeLocation);
	InNPC->SetActorHiddenInGame(false);
	InNPC->SetActorEnableCollision(true);
	InNPC->SetActorTickEnabled(true);

	// 스켈레탈 메쉬 틱 재활성화
	if (USkeletalMeshComponent* SkelMesh = InNPC->GetMesh())
	{
		SkelMesh->SetComponentTickEnabled(true);
	}

	// AI 상태 리셋
	if (ATPSNPCController* NPCCtrl = Cast<ATPSNPCController>(InNPC->GetController()))
	{
		NPCCtrl->ResetAIState();
	}

	// TargetSubsystem에 등록
	if (UTPSTargetSubsystem* TargetSS = GetWorld()->GetSubsystem<UTPSTargetSubsystem>())
	{
		TargetSS->RegisterNPC(InNPC);
	}
}

void UTPSNPCPoolSubsystem::DeferredSpawn()
{
	const int32 Remaining = MaxPoolSize - TotalSpawnedCount;
	if (Remaining <= 0)
	{
		UWorld* pWorld = GetWorld();
		if (pWorld)
		{
			pWorld->GetTimerManager().ClearTimer(DeferredSpawnTimerHandle);
		}
		UE_LOG(LogTemp, Log, TEXT("[NPCPool] Pool fully spawned: %d"), TotalSpawnedCount);
		return;
	}

	const int32 BatchCount = FMath::Min(DeferredSpawnBatchSize, Remaining);
	SpawnBatch(BatchCount);
}
