// Copyright (c) 2025 Amil Khisamov

#include "CombatComponent.h"

#include "Engine/SkeletalMeshSocket.h"

#include "MPShooter/Character/GunslingerCharacter.h"
#include "MPShooter/Weapon/Weapon.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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
	EquippedWeapon->ShowPickupWidget(false);
}
