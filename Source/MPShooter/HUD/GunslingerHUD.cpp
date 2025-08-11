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
		DrawCrosshairsTexture(CrosshairsPackage->CenterTexture, ViewportCenter);
		DrawCrosshairsTexture(CrosshairsPackage->LeftTexture, ViewportCenter);
		DrawCrosshairsTexture(CrosshairsPackage->RightTexture, ViewportCenter);
		DrawCrosshairsTexture(CrosshairsPackage->TopTexture, ViewportCenter);
		DrawCrosshairsTexture(CrosshairsPackage->BottomTexture, ViewportCenter);
	}
}

void AGunslingerHUD::DrawCrosshairsTexture(UTexture2D* Texture, const FVector2D& ViewportCenter)
{
	if (Texture)
	{
		const int32 TextureWidth = Texture->GetSizeX();
		const int32 TextureHeight = Texture->GetSizeY();
		const FVector2D TextureCenter(
			TextureWidth / 2.0f,
			TextureHeight / 2.0f
		);

		const FVector2D TextureDrawPoint = ViewportCenter - TextureCenter;

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
			FLinearColor::White
		);
	}
}
