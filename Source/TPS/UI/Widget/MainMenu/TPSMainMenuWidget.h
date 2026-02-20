#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TPSMainMenuWidget.generated.h"

/**
 * 메인 메뉴 위젯
 * - 게임 시작 / 게임 종료 버튼
 * - 버튼 클릭 콜백은 C++에서 바인딩, 레이아웃은 블루프린트에서 구성
 */
UCLASS()
class TPS_API UTPSMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 배경 동영상 MediaPlayer (에디터에서 지정) */
	UPROPERTY(EditDefaultsOnly, Category = "Media")
	TObjectPtr<class UMediaPlayer> BackgroundMediaPlayerAsset;

	/** 배경 동영상 MediaSource (에디터에서 지정) */
	UPROPERTY(EditDefaultsOnly, Category = "Media")
	TObjectPtr<class UMediaSource> BackgroundMediaSourceAsset;

	/** 게임 시작 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> StartGameButton;

	/** 게임 종료 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> QuitGameButton;

	/** 게임 시작 버튼 클릭 */
	UFUNCTION()
	void OnStartGameClicked();

	/** 게임 종료 버튼 클릭 */
	UFUNCTION()
	void OnQuitGameClicked();
};
