#include "UI/HUD/TPSHUD.h"
#include "UI/ViewModel/AmmoViewModel.h"
#include "UI/Widget/Ammo/TPSAmmoWidget.h"
#include "Pawn/Character/Player/TPSPlayer.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Canvas.h"

void ATPSHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* pPC = GetOwningPlayerController();
	if (!ensure(pPC)) return;

	ATPSPlayer* pPlayer = Cast<ATPSPlayer>(pPC->GetPawn());
	if (!ensure(pPlayer)) return;

	// ① 크로스헤어 뷰모델 초기화
	CrosshairViewModelInst = NewObject<UCrosshairViewModel>(this);
	if (ensure(CrosshairViewModelInst))
	{
		CrosshairViewModelInst->Initialize(pPlayer, CrosshairConfig);
	}

	// ② 탄약 위젯 생성 + Viewport 추가 (Collapsed)
	if (AmmoWidgetClass)
	{
		AmmoWidgetInst = CreateWidget<UTPSAmmoWidget>(pPC, AmmoWidgetClass);
		if (ensure(AmmoWidgetInst))
		{
			AmmoWidgetInst->AddToViewport();
			AmmoWidgetInst->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// ③ Player 델리게이트 바인딩
	if (!pPlayer->OnAmmoViewModelChangedDelegate.IsBoundToObject(this))
	{
		pPlayer->OnAmmoViewModelChangedDelegate.AddUObject(this, &ATPSHUD::OnAmmoViewModelChanged);
	}
}

void ATPSHUD::DrawHUD()
{
	Super::DrawHUD();
	DrawCrosshair();
	UpdateAmmoWidget();
}

void ATPSHUD::DrawCrosshair()
{
	if (!ensure(CrosshairViewModelInst)) return;

	// ① 플레이어 참조 유효성 확인 (무효 시 재초기화)
	if (!CrosshairViewModelInst->IsPlayerValid())
	{
		APlayerController* pPC = GetOwningPlayerController();
		if (!ensure(pPC)) return;

		ATPSPlayer* pPlayer = Cast<ATPSPlayer>(pPC->GetPawn());
		if (!pPlayer) return;

		CrosshairViewModelInst->Initialize(pPlayer, CrosshairConfig);
	}

	// ② 뷰모델 업데이트 (스프레드 보간)
	CrosshairViewModelInst->Update(GetWorld()->GetDeltaSeconds());

	if (!CrosshairViewModelInst->GetIsVisible()) return;

	// ③ 뷰모델에서 스타일 값 읽기
	const float CenterX = Canvas->ClipX * 0.5f;
	const float CenterY = Canvas->ClipY * 0.5f;

	const float SpreadOffset = CrosshairViewModelInst->GetSpreadOffset();
	const float LineLen = CrosshairViewModelInst->GetLineLength();
	const float Thickness = CrosshairViewModelInst->GetLineThickness();
	const FLinearColor Color = CrosshairViewModelInst->GetCrosshairColor();
	const float DotSize = CrosshairViewModelInst->GetCenterDotSize();

	// ④ 십자선 4방향 + 중앙 도트 드로잉
	DrawCrosshairLine(CenterX, CenterY,  0.f, -1.f, SpreadOffset, LineLen, Thickness, Color);
	DrawCrosshairLine(CenterX, CenterY,  0.f,  1.f, SpreadOffset, LineLen, Thickness, Color);
	DrawCrosshairLine(CenterX, CenterY, -1.f,  0.f, SpreadOffset, LineLen, Thickness, Color);
	DrawCrosshairLine(CenterX, CenterY,  1.f,  0.f, SpreadOffset, LineLen, Thickness, Color);

	DrawCenterDot(CenterX, CenterY, DotSize, Color);
}

void ATPSHUD::DrawCrosshairLine(float CenterX, float CenterY,
	float DirectionX, float DirectionY,
	float SpreadOffset, float LineLength,
	float Thickness, const FLinearColor& Color)
{
	const float StartX = CenterX + DirectionX * SpreadOffset;
	const float StartY = CenterY + DirectionY * SpreadOffset;
	const float EndX = StartX + DirectionX * LineLength;
	const float EndY = StartY + DirectionY * LineLength;

	Canvas->K2_DrawLine(FVector2D(StartX, StartY), FVector2D(EndX, EndY), Thickness, Color);
}

void ATPSHUD::DrawCenterDot(float CenterX, float CenterY,
	float DotSize, const FLinearColor& Color)
{
	const float HalfSize = DotSize * 0.5f;
	FCanvasTileItem DotItem(
		FVector2D(CenterX - HalfSize, CenterY - HalfSize),
		FVector2D(DotSize, DotSize),
		Color.ToFColor(true)
	);
	DotItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(DotItem);
}

void ATPSHUD::OnAmmoViewModelChanged(UAmmoViewModel* InAmmoViewModel)
{
	if (InAmmoViewModel)
	{
		// ① 장착 — ViewModel 참조 세팅 + Widget 표시
		AmmoViewModelRef = InAmmoViewModel;

		if (ensure(AmmoWidgetInst))
		{
			AmmoWidgetInst->SetVisibility(ESlateVisibility::HitTestInvisible);
			AmmoWidgetInst->UpdateAmmoDisplay(InAmmoViewModel->GetCurrentAmmo(), InAmmoViewModel->GetMaxAmmo(), InAmmoViewModel->GetAmmoColor());
		}
	}
	else
	{
		// ② 해제 — 참조 해제 + Widget 숨김
		AmmoViewModelRef.Reset();

		if (ensure(AmmoWidgetInst))
		{
			AmmoWidgetInst->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void ATPSHUD::UpdateAmmoWidget()
{
	if (!AmmoViewModelRef.IsValid()) return;
	if (!ensure(AmmoWidgetInst)) return;

	UAmmoViewModel* pViewModel = AmmoViewModelRef.Get();
	if (!ensure(pViewModel)) return;

	AmmoWidgetInst->UpdateAmmoDisplay(pViewModel->GetCurrentAmmo(), pViewModel->GetMaxAmmo(), pViewModel->GetAmmoColor());
}
