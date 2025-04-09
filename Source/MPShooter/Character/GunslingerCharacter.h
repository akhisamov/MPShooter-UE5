// Copyright (c) 2025 Amil Khisamov

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "GunslingerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class MPSHOOTER_API AGunslingerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGunslingerCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category=Camera)
	USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;
};
