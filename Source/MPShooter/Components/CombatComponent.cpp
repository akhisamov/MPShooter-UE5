// Copyright (c) 2025 Amil Khisamov

#include "CombatComponent.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "MPShooter/Character/GunslingerCharacter.h"
#include "MPShooter/Weapon/Weapon.h"

#define TRY_GET_CHARACTER_MOVEMENT Character; auto* CharacterMovement = Character->GetCharacterMovement()
#define GET_WALK_SPEED bIsAiming ? AimWalkSpeed : BaseWalkSpeed
#define SET_AIMING(bValue) bIsAiming = bValue; if (TRY_GET_CHARACTER_MOVEMENT) CharacterMovement->MaxWalkSpeed = GET_WALK_SPEED

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

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
}


void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed)
	{
		ServerFire();
	}
}

void UCombatComponent::ServerFire_Implementation()
{
	MulticastFire();
}

void UCombatComponent::MulticastFire_Implementation()
{
	if (Character && EquippedWeapon)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire();
	}
}
