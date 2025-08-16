// Copyright (c) 2025 Amil Khisamov

#include "CombatComponent.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

#include "MPShooter/Character/GunslingerCharacter.h"
#include "MPShooter/HUD/GunslingerHUD.h"
#include "MPShooter/PlayerController/GunslingerPlayerController.h"
#include "MPShooter/Weapon/Weapon.h"

#define TRY_GET_CHARACTER_MOVEMENT Character; auto* CharacterMovement = Character->GetCharacterMovement()
#define GET_WALK_SPEED bIsAiming ? AimWalkSpeed : BaseWalkSpeed
#define SET_AIMING(bValue) bIsAiming = bValue; if (TRY_GET_CHARACTER_MOVEMENT) CharacterMovement->MaxWalkSpeed = GET_WALK_SPEED

constexpr float TRACE_LENGTH = 80000.f;

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.0f;
	AimWalkSpeed = 400.0f;
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (TRY_GET_CHARACTER_MOVEMENT)
	{
		CharacterMovement->MaxWalkSpeed = BaseWalkSpeed;
	}
}

void UCombatComponent::SetAiming(bool bValue)
{
	SET_AIMING(bValue);
	ServerSetAiming(bValue);
}

void UCombatComponent::ServerSetAiming_Implementation(bool bValue)
{
	SET_AIMING(bValue);
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (TRY_GET_CHARACTER_MOVEMENT)
	{
		CharacterMovement->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (HUD && Character)
	{
		const FVector2D WalkSpeedRange = { 0.0f, Character->GetMovementComponent()->GetMaxSpeed() };
		FVector2D VelocityMultiplierRange = { 0.3f, 1.0f };
		if (bIsAiming)
		{
			VelocityMultiplierRange -= FVector2D::One() * VelocityMultiplierRange.X;
		}
		FVector Velocity = Character->GetVelocity();
		Velocity.Z = 0.0f;
		const float VelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

		if (Character->GetCharacterMovement()->IsFalling())
		{
			CrosshairsInAirSpreadFactor = FMath::FInterpTo(CrosshairsInAirSpreadFactor, 2.25f, DeltaTime, 2.25f);
		}
		else
		{
			CrosshairsInAirSpreadFactor = FMath::FInterpTo(CrosshairsInAirSpreadFactor, 0.0f, DeltaTime, 30.f);
		}

		HUD->SetCrosshairSpreadDelta(VelocityFactor + CrosshairsInAirSpreadFactor);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
}

void UCombatComponent::EquipWeapon(AWeapon* Weapon)
{
	if (Character == nullptr || Weapon == nullptr)
	{
		return;
	}

	EquippedWeapon = Weapon;
	EquippedWeapon->SetWeaponState(EWeaponState::Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName("RightHandSocket");
	if (HandSocket)
	{
		HandSocket->AttachActor(Weapon, Character->GetMesh());
	}
	EquippedWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;

	if (HUD)
	{
		HUD->SetCrosshairsHUDPackage(&EquippedWeapon->GetCrosshairsHUD());
	}
}


void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed)
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		ServerFire(HitResult.ImpactPoint);
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character && EquippedWeapon)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	const FVector2D CrosshairLocation = ViewportSize / 2.f;
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	const bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
	}
}

void UCombatComponent::SetController(AController* NewController)
{
	if (NewController->GetCharacter() == Character)
	{
		Controller = Cast<AGunslingerPlayerController>(NewController);
		HUD = Controller ? Cast<AGunslingerHUD>(Controller->GetHUD()) : nullptr;
	}
}
