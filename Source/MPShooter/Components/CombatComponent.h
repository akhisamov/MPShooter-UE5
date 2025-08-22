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

	void FireButtonPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

private:
	void SetController(AController* NewController);

	void UpdateHUDCrosshairs(float DeltaTime);

	float GetCrosshairVelocitySpreadFactor() const;
	void InterpCrosshairInAirSpreadFactor(float DeltaTime);
	void InterpCrosshairAimSpreadFactor(float DeltaTime);

	void InterpFOV(float DeltaTime);

private:
	AGunslingerCharacter* Character;
	class AGunslingerPlayerController* Controller = nullptr;
	class AGunslingerHUD* HUD = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bIsAiming = false;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed = false;

	float CrosshairInAirSpreadFactor = 0.0f;
	float CrosshairAimSpreadFactor = 0.0f;
	float CrosshairShootingFactor = 0.0f;

	FVector HitTarget;

	float DefaultFOV;
	float CurrentFOV;
};
