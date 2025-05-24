// Copyright (c) 2025 Amil Khisamov

#include "GunslingerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

#include "MPShooter/Weapon/Weapon.h"
#include "MPShooter/Components/CombatComponent.h"

static void SwitchOverlappingWeapons(AWeapon* LastWeapon, AWeapon* NewWeapon)
{
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
	if (NewWeapon)
	{
		NewWeapon->ShowPickupWidget(true);
	}
}

AGunslingerCharacter::AGunslingerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	TurningInPlace = ETurningInPlace::NotTurning;
}

void AGunslingerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AGunslingerCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void AGunslingerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

void AGunslingerCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	AWeapon* LastWeapon = OverlappingWeapon;
	OverlappingWeapon = Weapon && Weapon->GetOwner() ? nullptr : Weapon;
	if (IsLocallyControlled())
	{
		SwitchOverlappingWeapons(LastWeapon, OverlappingWeapon);
	}
}

bool AGunslingerCharacter::IsWeaponEquipped() const
{
	return Combat && Combat->EquippedWeapon;
}

bool AGunslingerCharacter::IsAiming() const
{
	return Combat && Combat->bIsAiming;
}

AWeapon* AGunslingerCharacter::GetEquippedWeapon() const
{
	return Combat ? Combat->EquippedWeapon : nullptr;
}

void AGunslingerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AGunslingerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		const FRotationMatrix YawRotationMatrix(YawRotation);
		const FVector ForwardDirection = YawRotationMatrix.GetUnitAxis(EAxis::X);
		const FVector RightDirection = YawRotationMatrix.GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AGunslingerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AGunslingerCharacter::EquipWeapon()
{
	if (OverlappingWeapon && Combat)
	{
		StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		AOYaw = 0.0f;
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

void AGunslingerCharacter::StartCrouching()
{
	Crouch();
}

void AGunslingerCharacter::StopCrouching()
{
	UnCrouch();
}

void AGunslingerCharacter::StartAiming()
{
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void AGunslingerCharacter::StopAiming()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void AGunslingerCharacter::AimOffset(float DeltaTime)
{
	if (Combat && !Combat->EquippedWeapon) return;
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	const float Speed = Velocity.Size();
	const bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.0f && !bIsInAir)
	{
		FRotator CurrentAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AOYaw = DeltaAimRotation.Yaw;
		bUseControllerRotationYaw = false;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.0f || bIsInAir)
	{
		StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		AOYaw = 0.0f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::NotTurning;
	}

	AOPitch = GetBaseAimRotation().Pitch;
	if (AOPitch > 90.0f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InputRange(270.0f, 360.0f);
		FVector2D OutputRange(-90.0f, 0.0f);
		AOPitch = FMath::GetMappedRangeValueClamped(InputRange, OutputRange, AOPitch);
	}
}

void AGunslingerCharacter::TurnInPlace(float DeltaTime)
{
	if (AOYaw < -90.0f)
	{
		TurningInPlace = ETurningInPlace::Left;
	}
	if (AOYaw > 90.0f)
	{
		TurningInPlace = ETurningInPlace::Right;
	}
}

void AGunslingerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AimOffset(DeltaTime);
}

void AGunslingerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (auto* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (auto* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGunslingerCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGunslingerCharacter::Look);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &AGunslingerCharacter::EquipWeapon);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AGunslingerCharacter::StartCrouching);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AGunslingerCharacter::StopCrouching);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AGunslingerCharacter::StartAiming);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AGunslingerCharacter::StopAiming);
	}
}

void AGunslingerCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	SwitchOverlappingWeapons(LastWeapon, OverlappingWeapon);
}

void AGunslingerCharacter::ServerEquipButtonPressed_Implementation()
{
	if (OverlappingWeapon && Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}
