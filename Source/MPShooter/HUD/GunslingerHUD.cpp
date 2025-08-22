// Copyright (c) 2025 Amil Khisamov


#include "GunslingerHUD.h"

void AGunslingerHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportCenter;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportCenter);
		ViewportCenter /= 2;
	}

	if (CrosshairsPackage)
	{
		DrawCrosshairsTexture(CrosshairsPackage->CenterTexture, ViewportCenter, FVector2D::ZeroVector);
		DrawCrosshairsTexture(CrosshairsPackage->LeftTexture, ViewportCenter, {-CrosshairSpread, 0.0f});
		DrawCrosshairsTexture(CrosshairsPackage->RightTexture, ViewportCenter, {CrosshairSpread, 0.0f});
		DrawCrosshairsTexture(CrosshairsPackage->TopTexture, ViewportCenter, {0.0f, -CrosshairSpread});
		DrawCrosshairsTexture(CrosshairsPackage->BottomTexture, ViewportCenter, {0.0f, CrosshairSpread});
	}
}

void AGunslingerHUD::UpdateCrosshairSpread(float SpreadDelta)
{
	CrosshairSpread = CrosshairSpreadMax * SpreadDelta;
}

void AGunslingerHUD::DrawCrosshairsTexture(UTexture2D* Texture, const FVector2D& ViewportCenter, const FVector2D& Spread)
{
	if (Texture)
	{
		const int32 TextureWidth = Texture->GetSizeX();
		const int32 TextureHeight = Texture->GetSizeY();
		const FVector2D TextureCenter(
			TextureWidth / 2.0f,
			TextureHeight / 2.0f
		);

		const FVector2D TextureDrawPoint = ViewportCenter - TextureCenter + Spread;

		DrawTexture(
			Texture,
			TextureDrawPoint.X,
			TextureDrawPoint.Y,
			TextureWidth,
			TextureHeight,
			0.0f,
			0.0f,
			1.0f,
			1.0f,
			CrosshairsColor
		);
	}
}
