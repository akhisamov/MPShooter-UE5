// Copyright (c) 2025 Amil Khisamov

#include "GunslingerHUD.h"

constexpr uint8 CROSSHAIRS_INIT_FLAG_TEXTURES = 1;
constexpr uint8 CROSSHAIRS_INIT_FLAG_SPREAD = 2;
constexpr uint8 CROSSHAIRS_INIT_FLAG_ALL =
	CROSSHAIRS_INIT_FLAG_TEXTURES | CROSSHAIRS_INIT_FLAG_SPREAD;

void AGunslingerHUD::DrawHUD()
{
	Super::DrawHUD();

	if ((CrosshairsInitStateFlags & CROSSHAIRS_INIT_FLAG_ALL) != CROSSHAIRS_INIT_FLAG_ALL)
	{
		return;
	}

	FVector2D ViewportCenter;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportCenter);
		ViewportCenter /= 2;
	}

	for (uint8 i = 0; i < Crosshair_Count; ++i)
	{
		UTexture2D* Texture = CrosshairTextures[i];
		const FVector2D& Spread = CrosshairSpreads[i];
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
}

void AGunslingerHUD::SetCrosshairTextures(const FCrosshairsHUDPackage& Package)
{
	CrosshairsInitStateFlags |= CROSSHAIRS_INIT_FLAG_TEXTURES;
	CrosshairTextures[Crosshair_Center] = Package.CenterTexture;
	CrosshairTextures[Crosshair_Left] = Package.LeftTexture;
	CrosshairTextures[Crosshair_Right] = Package.RightTexture;
	CrosshairTextures[Crosshair_Top] = Package.TopTexture;
	CrosshairTextures[Crosshair_Bottom] = Package.BottomTexture;
}

void AGunslingerHUD::UpdateCrosshairSpread(float SpreadDelta)
{
	CrosshairsInitStateFlags |= CROSSHAIRS_INIT_FLAG_SPREAD;
	const float Spread = CrosshairSpreadMax * SpreadDelta;

	CrosshairSpreads[Crosshair_Left] = { -Spread, 0.0f };
	CrosshairSpreads[Crosshair_Right] = { Spread, 0.0f };
	CrosshairSpreads[Crosshair_Top] = { 0.0f, -Spread };
	CrosshairSpreads[Crosshair_Bottom] = { 0.0f, Spread };
}
