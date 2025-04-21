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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(AWeapon* Weapon);

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bValue);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bValue);

	UFUNCTION()
	void OnRep_EquippedWeapon();

private:
	AGunslingerCharacter* Character;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bIsAiming = false;
};
