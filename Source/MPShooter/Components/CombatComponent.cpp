// Copyright (c) 2025 Amil Khisamov

#include "CombatComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include "MPShooter/Character/GunslingerCharacter.h"
#include "MPShooter/HUD/GunslingerHUD.h"
#include "MPShooter/PlayerController/GunslingerPlayerController.h"
#include "MPShooter/Weapon/Weapon.h"

#define TRY_GET_CHARACTER_MOVEMENT auto* CharacterMovement = Character ? Character->GetCharacterMovement() : nullptr
#define GET_WALK_SPEED bIsAiming ? AimWalkSpeed : BaseWalkSpeed
#define SET_AIMING(bValue) bIsAiming = bValue; if (TRY_GET_CHARACTER_MOVEMENT) CharacterMovement->MaxWalkSpeed = GET_WALK_SPEED

constexpr float TRACE_LENGTH = 80000.f;
constexpr float BASE_CROSSHAIR_SPREAD_FACTOR = 0.5f;

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.0f;
	AimWalkSpeed = 400.0f;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		if (auto* CharacterMovement = Character->GetCharacterMovement())
		{
			CharacterMovement->MaxWalkSpeed = BaseWalkSpeed;
		}

		if (auto* FollowCamera = Character->GetFollowCamera())
		{
			DefaultFOV = FollowCamera->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
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

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		UpdateHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
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
		HUD->SetCrosshairTextures(EquippedWeapon->GetCrosshairsHUD());
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

		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 1.f;
		}
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

		if (Character)
		{
			const float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
	}

	if (HUD)
	{
		AActor* TraceHitActor = TraceHitResult.GetActor();
		if (TraceHitActor && TraceHitActor->Implements<UInteractWithCrosshairsInterface>())
		{
			HUD->SetCrosshairsColor(FLinearColor::Red);
		}
		else
		{
			HUD->SetCrosshairsColor(FLinearColor::White);
		}
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

void UCombatComponent::UpdateHUDCrosshairs(float DeltaTime)
{
	InterpCrosshairInAirSpreadFactor(DeltaTime);
	InterpCrosshairAimSpreadFactor(DeltaTime);
	CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 15.f);

	if (HUD)
	{
		const float VelocityFactor = GetCrosshairVelocitySpreadFactor();
		const float CrosshairSpread =
			BASE_CROSSHAIR_SPREAD_FACTOR +
			VelocityFactor +
			CrosshairInAirSpreadFactor -
			CrosshairAimSpreadFactor +
			CrosshairShootingFactor;
		HUD->UpdateCrosshairSpread(CrosshairSpread);
	}
}

float UCombatComponent::GetCrosshairVelocitySpreadFactor() const
{
	if (TRY_GET_CHARACTER_MOVEMENT)
	{
		const FVector2D WalkSpeedRange = { 0.0f, CharacterMovement->GetMaxSpeed() };
		const FVector2D VelocityMultiplierRange = { 0.0f, 1.0f };
		FVector Velocity = Character->GetVelocity();
		Velocity.Z = 0.0f;
		return FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
	}
	return 0.f;
}

void UCombatComponent::InterpCrosshairInAirSpreadFactor(float DeltaTime)
{
	const bool isFalling = Character && Character->GetCharacterMovement()
		&& Character->GetCharacterMovement()->IsFalling();
	const float Target = isFalling ? 2.25f : 0.f;
	const float Speed = isFalling ? 2.25f : 30.f;
	CrosshairInAirSpreadFactor = FMath::FInterpTo(CrosshairInAirSpreadFactor, Target, DeltaTime, Speed);
}

void UCombatComponent::InterpCrosshairAimSpreadFactor(float DeltaTime)
{
	const float Target = bIsAiming ? 0.58f : 0.f;
	CrosshairAimSpreadFactor = FMath::FInterpTo(CrosshairAimSpreadFactor, Target, DeltaTime, 30.f);
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr)
		return;

	if (bIsAiming)
	{
		CurrentFOV = FMath::FInterpTo(
			CurrentFOV,
			EquippedWeapon->GetZoomedFOV(),
			DeltaTime,
			EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(
			CurrentFOV,
			DefaultFOV,
			DeltaTime,
			EquippedWeapon->GetZoomInterpSpeed());
	}

	if (auto* FollowCamera = Character ? Character->GetFollowCamera() : nullptr)
	{
		FollowCamera->SetFieldOfView(CurrentFOV);
	}
}
