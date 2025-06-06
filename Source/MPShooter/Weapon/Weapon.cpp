// Copyright (c) 2025 Amil Khisamov

#include "Weapon.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "MPShooter/Character/GunslingerCharacter.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);

	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereBeginOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

void AWeapon::ShowPickupWidget(bool bShow)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShow);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (WeaponMesh && FireEffectMuzzle)
	{
		const FTransform MuzzleTransform = WeaponMesh->GetSocketTransform("Muzzle");
		UNiagaraComponent* EffectComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			FireEffectMuzzle,
			WeaponMesh,
			NAME_None,
			MuzzleTransform.GetLocation(),
			FRotator(MuzzleTransform.GetRotation()),
			EAttachLocation::KeepWorldPosition,
			true
		);
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	UpdateWeaponState();
}

void AWeapon::OnSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherCmp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (PickupWidget)
	{
		auto* GunslingerCharacter = Cast<AGunslingerCharacter>(OtherActor);
		if (GunslingerCharacter)
		{
			GunslingerCharacter->SetOverlappingWeapon(this);
		}
	}
}

void AWeapon::OnSphereEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (PickupWidget)
	{
		auto* GunslingerCharacter = Cast<AGunslingerCharacter>(OtherActor);
		if (GunslingerCharacter)
		{
			GunslingerCharacter->SetOverlappingWeapon(nullptr);
		}
	}
}

void AWeapon::OnRep_WeaponState()
{
	UpdateWeaponState();
}

void AWeapon::UpdateWeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::Equipped:
	{
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}
	}
}
