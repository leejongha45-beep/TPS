#include "Animation/Notify/TPSAnimNotify_EquipSound.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UTPSAnimNotify_EquipSound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;
	if (!ensure(EquipSoundAsset.Get())) return;

	// 캐릭터 위치에서 3D 사운드 재생
	const FVector SoundLocation = MeshComp->GetOwner()->GetActorLocation();

	UGameplayStatics::PlaySoundAtLocation(
		MeshComp->GetWorld(), EquipSoundAsset.Get(), SoundLocation);
}
