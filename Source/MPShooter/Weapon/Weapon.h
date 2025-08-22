// Copyright (c) 2025 Amil Khisamov

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MPShooter/HUD/CrosshairsHUDPackage.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Inital UMETA(DisplayName = "Initial State"),
	Equipped UMETA(DisplayName = "Equipped"),
	Dropped UMETA(DisplayName = "Dropped"),

	MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class MPSHOOTER_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ShowPickupWidget(bool bShow);

	virtual void Fire(const FVector& HitTarget);

	void SetWeaponState(EWeaponState State);

	FORCEINLINE UStaticMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	FORCEINLINE const FCrosshairsHUDPackage& GetCrosshairsHUD() const { return CrosshairsHUD; }

	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherCmp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere)
	EWeaponState WeaponState = EWeaponState::Inital;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere)
	class UWidgetComponent* PickupWidget;

	void UpdateWeaponState();

	UPROPERTY(EditAnywhere, Category = "Effects")
	class UNiagaraSystem* FireEffectMuzzle;

	UPROPERTY(EditAnywhere)
	class USoundCue* GunshotSound;

	UPROPERTY(EditAnywhere)
	FCrosshairsHUDPackage CrosshairsHUD;

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;
};
