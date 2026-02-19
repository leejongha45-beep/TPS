#include "Animation/Notify/TPSAnimNotify_FireSound.h"
#include "Kismet/GameplayStatics.h"

void UTPSAnimNotify_FireSound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;
	if (!ensure(FireSoundAsset)) return;

	// ① Muzzle 소켓 위치에서 3D 사운드 재생
	const FVector SoundLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

	UGameplayStatics::PlaySoundAtLocation(
		MeshComp->GetWorld(), FireSoundAsset, SoundLocation);
}
