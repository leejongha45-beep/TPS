#include "TPSMainMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MediaPlayer.h"

void UTPSMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// ① 버튼 클릭 바인딩
	if (ensure(StartGameButton.Get()))
	{
		StartGameButton->OnClicked.AddDynamic(this, &UTPSMainMenuWidget::OnStartGameClicked);
	}

	if (ensure(QuitGameButton.Get()))
	{
		QuitGameButton->OnClicked.AddDynamic(this, &UTPSMainMenuWidget::OnQuitGameClicked);
	}

	// ② 배경 동영상 루프 재생
	if (ensure(BackgroundMediaPlayerAsset.Get()) && ensure(BackgroundMediaSourceAsset.Get()))
	{
		BackgroundMediaPlayerAsset->SetLooping(true);
		BackgroundMediaPlayerAsset->OpenSource(BackgroundMediaSourceAsset.Get());
	}
}

void UTPSMainMenuWidget::NativeDestruct()
{
	if (ensure(BackgroundMediaPlayerAsset.Get()))
	{
		BackgroundMediaPlayerAsset->Close();
	}

	Super::NativeDestruct();
}

void UTPSMainMenuWidget::OnStartGameClicked()
{
	// ② Village 맵으로 전환
	UGameplayStatics::OpenLevel(this, FName(TEXT("Village")));
}

void UTPSMainMenuWidget::OnQuitGameClicked()
{
	// ③ 게임 종료
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
}