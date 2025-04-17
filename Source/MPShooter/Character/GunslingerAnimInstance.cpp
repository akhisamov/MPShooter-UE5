// Copyright (c) 2025 Amil Khisamov


#include "GunslingerAnimInstance.h"
#include "GunslingerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UGunslingerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	GunslingerCharacter = Cast<AGunslingerCharacter>(TryGetPawnOwner());
}

void UGunslingerAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (GunslingerCharacter == nullptr)
	{
		GunslingerCharacter = Cast<AGunslingerCharacter>(TryGetPawnOwner());
	}
	if (GunslingerCharacter == nullptr)
	{
		return;
	}

	FVector Velocity = GunslingerCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = GunslingerCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = GunslingerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.0f;
	bWeaponEquipped = GunslingerCharacter->IsWeaponEquipped();
}
