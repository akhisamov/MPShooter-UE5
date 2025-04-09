// Copyright (c) 2025 Amil Khisamov

#include "GunslingerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

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
}

void AGunslingerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AGunslingerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (PlayerInputComponent)
	{
		PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

		PlayerInputComponent->BindAxis("MoveForward", this, &AGunslingerCharacter::MoveForward);
		PlayerInputComponent->BindAxis("Strafe", this, &AGunslingerCharacter::Strafe);
		PlayerInputComponent->BindAxis("LookUp", this, &AGunslingerCharacter::LookUp);
		PlayerInputComponent->BindAxis("Turn", this, &AGunslingerCharacter::Turn);
	}
}

void AGunslingerCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AGunslingerCharacter::Strafe(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AGunslingerCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AGunslingerCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AGunslingerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
