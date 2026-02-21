#include "Actor/ItemBox/TPSItemBox.h"
#include "Actor/ItemBox/TPSItemBoxInteractionComponent.h"
#include "Character/Component/Action/TPSEquipComponent.h"
#include "Weapon/TPSWeaponBase.h"
#include "Character/Component/Action/TPSPlayerInteractionComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/Player/TPSPlayer.h"

DEFINE_LOG_CATEGORY(LogItemBox);

ATPSItemBox::ATPSItemBox()
{
	PrimaryActorTick.bCanEverTick = false;

	// ① 메시 컴포넌트
	if (!MeshInst)
	{
		MeshInst = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
		if (ensure(MeshInst))
		{
			SetRootComponent(MeshInst);
		}
	}

	// ② 트리거 박스 + 오버랩 델리게이트 바인딩
	if (!BoxCollisionInst)
	{
		BoxCollisionInst = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
		if (ensure(BoxCollisionInst))
		{
			BoxCollisionInst->SetupAttachment(RootComponent);
			BoxCollisionInst->SetCollisionProfileName(TEXT("Trigger"));
			BoxCollisionInst->SetBoxExtent(FVector(150.f, 150.f, 100.f));

			if (!BoxCollisionInst->OnComponentBeginOverlap.IsBound())
			{
				BoxCollisionInst->OnComponentBeginOverlap.AddDynamic(this, &ATPSItemBox::OnBoxBeginOverlap);
			}

			if (!BoxCollisionInst->OnComponentEndOverlap.IsBound())
			{
				BoxCollisionInst->OnComponentEndOverlap.AddDynamic(this, &ATPSItemBox::OnBoxEndOverlap);
			}
		}
	}

	// ③ 상호작용 컴포넌트
	if (!InteractionComponentInst)
	{
		InteractionComponentInst = CreateDefaultSubobject<UTPSItemBoxInteractionComponent>(TEXT("InteractionComponent"));
		ensure(InteractionComponentInst);
	}
}

void ATPSItemBox::SpawnWeapon(EWeaponType TargetWeapon)
{
	int32 ItemIndex = static_cast<int32>(TargetWeapon);

	if (ensure(ItemClassArray.IsValidIndex(ItemIndex)))
	{
		if (!ensure(ItemClassArray[ItemIndex])) return;

		if (!SpawnedWeapon)
		{
			// ① 무기 액터 스폰
			ATPSPlayer* pPlayer = Cast<ATPSPlayer>(InteractableInterface.GetObject());
			if (!ensure(pPlayer)) return;

			SpawnedWeapon = GetWorld()->SpawnActor<ATPSWeaponBase>(ItemClassArray[ItemIndex]);
			if (!ensure(SpawnedWeapon)) return;

			// ② 비장착 소켓에 부착
			SpawnedWeapon->Detach(pPlayer->GetMesh());

			// ③ EquipComponent에 무기 인터페이스 등록
			UTPSEquipComponent* pEquipComponent = pPlayer->GetEquipComponent();
			if (!ensure(pEquipComponent)) return;

			pEquipComponent->SetWeaponInterface(SpawnedWeapon);
		}
	}
}

void ATPSItemBox::Interact()
{
	if (ensure(InteractionComponentInst))
	{
		if (ensure(InteractableInterface))
		{
			ATPSPlayer* pPlayer = Cast<ATPSPlayer>(InteractableInterface.GetObject());
			if (!ensure(pPlayer)) return;

			APlayerController* pController = Cast<APlayerController>(pPlayer->GetController());
			InteractionComponentInst->ToggleItemBox(!InteractionComponentInst->GetIsOpen(), pController);
		}
	}
}

void ATPSItemBox::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                    bool bFromSweep, const FHitResult& SweepResult)
{
	ATPSPlayer* pPlayer = Cast<ATPSPlayer>(OtherActor);
	if (!ensure(pPlayer)) return;

	// ① 상호작용 대상 인터페이스 캐싱
	InteractableInterface = pPlayer;
	ensure(InteractableInterface);

	// ② 플레이어의 InteractionComponent에 자신을 타겟으로 등록
	UTPSPlayerInteractionComponent* pInteractionComponent = pPlayer->GetInteractionComponent();
	if (ensure(pInteractionComponent))
	{
		pInteractionComponent->SetCurrentTarget(this);
	}

	UE_LOG(LogItemBox, Log, TEXT("[OnBoxBeginOverlap] Player entered trigger zone"));
}

void ATPSItemBox::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ATPSPlayer* pPlayer = Cast<ATPSPlayer>(OtherActor);
	if (!pPlayer) return;

	UTPSPlayerInteractionComponent* pInteractionComponent = pPlayer->GetInteractionComponent();
	if (ensure(pInteractionComponent))
	{
		pInteractionComponent->ClearCurrentTarget(this);
	}

	InteractableInterface = nullptr;
	ensure(!InteractableInterface);

	UE_LOG(LogItemBox, Log, TEXT("[OnBoxEndOverlap] Player left trigger zone"));
}