#include "UI/HUD/TPSHUD.h"
#include "Pawn/Character/Player/TPSPlayer.h"
#include "Engine/Canvas.h"

void ATPSHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* pPC = GetOwningPlayerController();
	if (!ensure(pPC)) return;

	ATPSPlayer* pPlayer = Cast<ATPSPlayer>(pPC->GetPawn());
	if (!ensure(pPlayer)) return;

	CrosshairViewModelInst = NewObject<UCrosshairViewModel>(this);
	if (ensure(CrosshairViewModelInst))
	{
		CrosshairViewModelInst->Initialize(pPlayer, CrosshairConfig);
	}
}

void ATPSHUD::DrawHUD()
{
	Super::DrawHUD();
	DrawCrosshair();
}

void ATPSHUD::DrawCrosshair()
{
	if (!ensure(CrosshairViewModelInst)) return;

	if (!CrosshairViewModelInst->IsPlayerValid())
	{
		APlayerController* pPC = GetOwningPlayerController();
		if (!ensure(pPC)) return;

		ATPSPlayer* pPlayer = Cast<ATPSPlayer>(pPC->GetPawn());
		if (!pPlayer) return;

		CrosshairViewModelInst->Initialize(pPlayer, CrosshairConfig);
	}

	CrosshairViewModelInst->Update(GetWorld()->GetDeltaSeconds());

	if (!CrosshairViewModelInst->GetIsVisible()) return;

	const float CenterX = Canvas->ClipX * 0.5f;
	const float CenterY = Canvas->ClipY * 0.5f;

	const float SpreadOffset = CrosshairViewModelInst->GetSpreadOffset();
	const float LineLen = CrosshairViewModelInst->GetLineLength();
	const float Thickness = CrosshairViewModelInst->GetLineThickness();
	const FLinearColor Color = CrosshairViewModelInst->GetCrosshairColor();
	const float DotSize = CrosshairViewModelInst->GetCenterDotSize();

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
