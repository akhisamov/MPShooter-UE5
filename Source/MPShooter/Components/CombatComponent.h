// Copyright (c) 2025 Amil Khisamov

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AGunslingerCharacter;
class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MPSHOOTER_API UCombatComponent : public UActorComponent
{
	friend AGunslingerCharacter;
	GENERATED_BODY()

public:
	UCombatComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void EquipWeapon(AWeapon* Weapon);

protected:
	virtual void BeginPlay() override;

private:
	AGunslingerCharacter* Character;
	AWeapon* EquippedWeapon;
};
