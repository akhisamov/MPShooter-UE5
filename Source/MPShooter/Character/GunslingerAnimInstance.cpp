// Copyright (c) 2025 Amil Khisamov


#include "GunslingerAnimInstance.h"
#include "GunslingerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MPShooter/Weapon/Weapon.h"

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
	bIsCrouched = GunslingerCharacter->bIsCrouched;
	bIsAiming = GunslingerCharacter->IsAiming();
	EquippedWeapon = GunslingerCharacter->GetEquippedWeapon();
	TurningInPlace = GunslingerCharacter->GetTurningInPlace();

	const FRotator AimRotation = GunslingerCharacter->GetBaseAimRotation();
	const FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(GunslingerCharacter->GetVelocity());
	const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = GunslingerCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AOYaw = GunslingerCharacter->GetAimOffsetYaw();
	AOPitch = GunslingerCharacter->GetAimOffsetPitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && GunslingerCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform("LeftHandSocket");
		FVector OutPosition;
		FRotator OutRotation;
		GunslingerCharacter->GetMesh()->TransformToBoneSpace(
			"hand_r",
			LeftHandTransform.GetLocation(),
			LeftHandTransform.GetRotation().Rotator(),
			OutPosition,
			OutRotation
		);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (bLocallyControlled)
		{
			const FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform("Hand_R");
			const FVector RightHandLocation = RightHandTransform.GetLocation();
			RightHandRotation = UKismetMathLibrary::FindLookAtRotation(RightHandLocation, RightHandLocation + (RightHandLocation - GunslingerCharacter->GetHitTarget()));
		}

		if (bDrawDebugLineToHitTarget)
		{
			const FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform("Muzzle");
			const FVector MuzzleY(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::Y));
			DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleY * 1000.f, FColor::Red);
			DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), GunslingerCharacter->GetHitTarget(), FColor::Green);
		}
	}
}

void UGunslingerAnimInstance::NativeBeginPlay()
{
	bLocallyControlled = GunslingerCharacter ? GunslingerCharacter->IsLocallyControlled() : false;
}
