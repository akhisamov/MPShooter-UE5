// Copyright (c) 2025 Amil Khisamov

#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GunslingerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "MPShooter/Components/CombatComponent.h"
#include "MPShooter/Weapon/Weapon.h"

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
}

void AGunslingerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AGunslingerCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void AGunslingerCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	AWeapon* LastWeapon = OverlappingWeapon;
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		SwitchOverlappingWeapons(LastWeapon, OverlappingWeapon);
	}
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
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Cyan,
			TEXT("EquipWeapon")
		);
	}
}

void AGunslingerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	}
}

void AGunslingerCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	SwitchOverlappingWeapons(LastWeapon, OverlappingWeapon);
}
