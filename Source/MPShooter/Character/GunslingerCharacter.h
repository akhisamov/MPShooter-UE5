// Copyright (c) 2025 Amil Khisamov

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "GunslingerCharacter.generated.h"

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

	void MoveForward(float Value);
	void Strafe(float Value);
	void LookUp(float Value);
	void Turn(float Value);

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = HUD, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;
};
