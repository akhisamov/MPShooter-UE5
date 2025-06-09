// Copyright (c) 2025 Amil Khisamov

#include "ProjectileWeapon.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	// Server-only projectile spawn
	if (!HasAuthority()) return;

	auto* InstigatorPawn = Cast<APawn>(GetOwner());
	const FTransform MuzzleTransform = GetWeaponMesh()->GetSocketTransform("Muzzle");
	const FVector ToTarget = HitTarget - MuzzleTransform.GetLocation();
	const FRotator TargetRotation = ToTarget.Rotation();
	if (ProjectileClass && InstigatorPawn)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				ProjectileClass,
				MuzzleTransform.GetLocation(),
				TargetRotation,
				SpawnParams
			);
		}
	}
}
