// Copyright (c) 2025 Amil Khisamov

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "MPShooter/HUD/CrosshairsHUDPackage.h"

#include "GunslingerHUD.generated.h"

enum CrosshairType : uint8
{
	Crosshair_Center,
	Crosshair_Left,
	Crosshair_Right,
	Crosshair_Top,
	Crosshair_Bottom,

	Crosshair_Count
};

UCLASS()
class MPSHOOTER_API AGunslingerHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	FORCEINLINE void SetCrosshairsColor(const FLinearColor& Color) { CrosshairsColor = Color; }

	void SetCrosshairTextures(const FCrosshairsHUDPackage& Package);

	void UpdateCrosshairSpread(float SpreadDelta);

private:
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	uint8 CrosshairsInitStateFlags = 0;
	UTexture2D* CrosshairTextures[Crosshair_Count];
	FVector2D CrosshairSpreads[Crosshair_Count] = { FVector2D(0) };

	FLinearColor CrosshairsColor = FLinearColor::White;
};
