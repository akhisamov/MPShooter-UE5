// Copyright (c) 2025 Amil Khisamov

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class MPSHOOTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetDisplayText(const FString& TextToDisplay);

	UFUNCTION(BlueprintCallable)
	void ShowPlayerInfo(APawn* InPawn);

protected:
	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"))
	class UTextBlock* DisplayText;
};
