// Copyright (c) 2025 Amil Khisamov

#pragma once

#include "CrosshairsHUDPackage.generated.h"

USTRUCT(BlueprintType)
struct FCrosshairsHUDPackage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	class UTexture2D* CenterTexture = nullptr;
	UPROPERTY(EditAnywhere)
	class UTexture2D* LeftTexture = nullptr;
	UPROPERTY(EditAnywhere)
	UTexture2D* RightTexture = nullptr;
	UPROPERTY(EditAnywhere)
	UTexture2D* TopTexture = nullptr;
	UPROPERTY(EditAnywhere)
	UTexture2D* BottomTexture = nullptr;
};
