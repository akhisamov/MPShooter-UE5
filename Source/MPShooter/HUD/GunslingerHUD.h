// Copyright (c) 2025 Amil Khisamov

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MPShooter/HUD/CrosshairsHUDPackage.h"
#include "GunslingerHUD.generated.h"

UCLASS()
class MPSHOOTER_API AGunslingerHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	FORCEINLINE void SetCrosshairsHUDPackage(const FCrosshairsHUDPackage* Package) { CrosshairsPackage = Package; }

	void UpdateCrosshairSpread(float SpreadDelta);

private:
	void DrawCrosshairsTexture(UTexture2D* Texture, const FVector2D& ViewportCenter, const FVector2D& Spread);

private:
	const FCrosshairsHUDPackage* CrosshairsPackage = nullptr;

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	float CrosshairSpread = 0.0f;
};
