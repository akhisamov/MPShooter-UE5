// Copyright (c) 2025 Amil Khisamov


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

static FString NetRoleToString(ENetRole Role)
{
	switch (Role)
	{
	case ENetRole::ROLE_Authority:
		return "Authority";
	case ENetRole::ROLE_AutonomousProxy:
		return "AutonomousProxy";
	case ENetRole::ROLE_SimulatedProxy:
		return "SimulatedProxy";
	case ENetRole::ROLE_None:
		return "None";
	}
	return "";
}

void UOverheadWidget::SetDisplayText(const FString& TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerInfo(APawn* InPawn)
{
	if (!InPawn)
	{
		return;
	}

	const ENetRole RemoteRole = InPawn->GetRemoteRole();
	const FString RemoteRoleString = NetRoleToString(RemoteRole);

	FString Name;
	APlayerState* PlayerState = InPawn->GetPlayerState();
	if (PlayerState)
	{
		Name = PlayerState->GetPlayerName();
	}

	SetDisplayText(FString::Printf(TEXT("%s [%s]"), *Name, *RemoteRoleString));
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}
