// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

void UShooterAnimInstance::UpdateAnimationProperties(float Deltatime)
{
	if (ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}

	if (ShooterCharacter)
	{
		FVector Velocity{ShooterCharacter->GetVelocity()};
		Velocity.Z = 0;
		Speed = Velocity.Size();

		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

		bIsMoving = ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;

		const FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		const FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		
		if(ShooterCharacter->GetVelocity().Size() > 0) LastMovementOffsetYaw = MovementOffsetYaw;

		bIsAiming = ShooterCharacter->GetIsAiming();
	}
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}
