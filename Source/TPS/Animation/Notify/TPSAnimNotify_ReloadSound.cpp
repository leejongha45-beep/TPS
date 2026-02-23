#include "Animation/Notify/TPSAnimNotify_ReloadSound.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UTPSAnimNotify_ReloadSound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;
	if (!ensure(ReloadSoundAsset.Get())) return;

	// 캐릭터 위치에서 3D 사운드 재생
	const FVector SoundLocation = MeshComp->GetOwner()->GetActorLocation();

	UGameplayStatics::PlaySoundAtLocation(
		MeshComp->GetWorld(), ReloadSoundAsset.Get(), SoundLocation);
}
