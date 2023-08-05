// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Item.h"
#include "Weapon.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter() :
	//Base Rates
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),

	//Turn rates for aiming/not aiming
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),

	//Mouse look sensitivity scale factors
	MouseHipTurnRate(1.f),
	MouseHipLookUpRate(1.f),
	MouseAimingLookUpRate(0.2f),
	MouseAimingTurnRate(0.2f),
	bIsAiming(false),

	//Camera FOV
	CameraDefaultFOV(0.f),
	CameraZoomedFOV(35.f),
	CameraCurrentFOV(0.f),
	ZoomInterpolationSpeed(20.f),

	//Crosshair spread factors
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimingFactor(0.f),
	CrosshairShootingFactor(0.f),

	//Bullet fire timer variables
	ShootTimeDuration(0.05f),
	bFiringBullet(false),

	//Fire rates
	bFireButtonIsPressed(false),
	bAbleToFire(true),
	AutomaticFireRate(0.1f),

	//Trace variables
	bShouldTraceForItems(false)


{
	PrimaryActorTick.bCanEverTick = true;

	//CAMERA ARM
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent); //Camera follows the player at this distance behind the controller
	CameraBoom->TargetArmLength = 180.f; //Rotate the arm based on the controller
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	//CAMERA
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); //Attach camera to end of CameraBoom
	FollowCamera->bUsePawnControlRotation = false; // Stop camera from rotate relative to the arm

	//USE OF CONTROLLER ROTATION
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	//CHARACTER MOVEMENT
	GetCharacterMovement()->bOrientRotationToMovement = true; //Character moves in the direction of input
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = .2f;
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	OverlappedItemCount = OverlappedItemCount + Amount <= 0 ? 0 : OverlappedItemCount + Amount;
	bShouldTraceForItems = OverlappedItemCount <= 0 ? false : true;
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}
	
	EquipWeapon(SpawnDefaultWeapon());
}

void AShooterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ZoomInterpolation(DeltaSeconds);

	SetLookUpRates();

	CalculateCrosshairSpread(DeltaSeconds);

	TraceForItems();
}

#pragma region UTILITY METHODS

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);

	if (bCrosshairHit)
	{
		//Possible hit location between crosshair and target-> Check if there is something between barrel and target 
		OutBeamLocation = CrosshairHitResult.Location;
	}

	//Perform the second trace, from barrel to check if there is a blocking object
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart{MuzzleSocketLocation};
	const FVector StartToEnd{OutBeamLocation - MuzzleSocketLocation};
	const FVector WeaponTraceEnd{MuzzleSocketLocation + StartToEnd * 1.25f};
	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECC_Visibility);

	//If there is an object between the barrel and beam and the CrossHairs are not exact -> change the end point
	if (WeaponTraceHit.bBlockingHit)
	{
		OutBeamLocation = WeaponTraceHit.Location;
		return true;
	}
	return false;
}
#pragma endregion

#pragma region INPUT METHODS

void AShooterCharacter::MoveForward(const float Value)
{
	if (Controller != nullptr && Value != 0)
	{
		//Find out which way is Forward
		const FRotator Rotation{Controller->GetControlRotation()};
		const FRotator YawRotation{0, Rotation.Yaw, 0};

		const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X)};
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(const float Value)
{
	if (Controller != nullptr && Value != 0)
	{
		//Find out which way is Right
		const FRotator Rotation{Controller->GetControlRotation()};
		const FRotator YawRotation{0, Rotation.Yaw, 0};

		const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y)};
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::FireWeapon()
{
	//Play fire sounds
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	if (const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket"))
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		if (MuzzleFlash_Particles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash_Particles, SocketTransform);
		}

		FVector BeamEnd;

		//Detects which is the correct end location for the bullet from CrossHairs-target and checking if there is an obstacle between them
		if (GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd))
		{
			if (BulletHit_Particles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletHit_Particles, BeamEnd);
			}
			if (SmokeTrail)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(), SmokeTrail, SocketTransform);
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}

		//Start the spread from firing the weapon
		StartCrosshairBulletFire();
	}

	//Play the animation montage of firing
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::AimingButtonPressed()
{
	bIsAiming = true;
	GetFollowCamera()->SetFieldOfView(CameraZoomedFOV);
}

void AShooterCharacter::AimingButtonReleased()
{
	bIsAiming = false;
	GetFollowCamera()->SetFieldOfView(CameraDefaultFOV);
}

void AShooterCharacter::ZoomInterpolation(const float DeltaTime)
{
	CameraCurrentFOV = bIsAiming
		                   ? FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpolationSpeed)
		                   : FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpolationSpeed);

	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::SetLookUpRates()
{
	BaseTurnRate = bIsAiming ? AimingTurnRate : HipTurnRate;
	BaseLookUpRate = bIsAiming ? AimingLookUpRate : HipLookUpRate;
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	//Calculate Move Spread Factor
	const FVector2D WalkSpeedRange{0.f, 600.f};
	const FVector2D VelocityMultiplierRange{0.f, 1.f};
	FVector Velocity{GetVelocity()};
	Velocity.Z = 0.f;

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange,
	                                                            Velocity.Size());

	//Calculate In Air Spread Factor
	const bool bIsCharacterGrounded = !GetCharacterMovement()->IsFalling();
	const float JumpInterpSpeed = bIsCharacterGrounded ? 30.f : 2.25f;
	const float JumpTargetValue = bIsCharacterGrounded ? 0.f : 2.25f;

	CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, JumpTargetValue, DeltaTime, JumpInterpSpeed);

	//Calculate Aiming spread factor

	const float AimInterpSpeed = 30.f;
	const float AimTargetValue = bIsAiming ? 0.6f : 0.f;

	CrosshairAimingFactor = FMath::FInterpTo(CrosshairAimingFactor, AimTargetValue, DeltaTime, AimInterpSpeed);

	//Calculate Shooting spread factor (true 0.05 seconds after firing)
	const float ShootInterpSpeed = 60.f;
	const float ShootTargetValue = bFiringBullet ? 0.3f : 0.f;

	CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, ShootTargetValue, DeltaTime, ShootInterpSpeed);

	CrosshairSpreadMultiplier = 0.5f +
		CrosshairVelocityFactor +
		CrosshairInAirFactor -
		CrosshairAimingFactor +
		CrosshairShootingFactor;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonIsPressed = true;
	StartFireTimer();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonIsPressed = false;
}

//Called when FireButton is pressed or in AutoFireReset if we still pressing fire button
void AShooterCharacter::StartFireTimer()
{
	if (bAbleToFire)
	{
		FireWeapon();
		bAbleToFire = false;
		GetWorldTimerManager().SetTimer(
			AutoFireTimer,
			this,
			&AShooterCharacter::AutoFireReset,
			AutomaticFireRate);
	}
}

//Called as callback when the StartFireTimer is over
void AShooterCharacter::AutoFireReset()
{
	bAbleToFire = true;
	if (bFireButtonIsPressed)
	{
		StartFireTimer();
	}
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	//Get viewport size
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	//Get Screen location of Crosshair
	const FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	//Get world position and direction of CrossHairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this,
		                                                               0),
	                                                               CrosshairLocation,
	                                                               CrosshairWorldPosition,
	                                                               CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		const FVector Start{CrosshairWorldPosition};
		const FVector End{Start + CrosshairWorldDirection * 50'000.f};
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECC_Visibility);

		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}
	return false;
}

void AShooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceResult, HitLocation);

		if (ItemTraceResult.bBlockingHit)
		{
			AItem* HitItem = Cast<AItem>(ItemTraceResult.GetActor());
			
			//Ensure Cast success and widget availability to show it
			if (HitItem && HitItem->GetPickupWidget())
			{
				if(HitItem->GetItemTraceability())
				{
					HitItem->GetPickupWidget()->SetVisibility(true);
				}
			}

			if (LastItemTraced)
			{
				if (HitItem != LastItemTraced)
				{
					//We stopped looking the item to look another different
					LastItemTraced->GetPickupWidget()->SetVisibility(false);
				}
			}
			LastItemTraced = HitItem;
		}
		else if (LastItemTraced)
		{
			//No longer tracing items, disable widgets
			LastItemTraced->GetPickupWidget()->SetVisibility(false);
		}
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon() const
{
	if(!DefaultWeaponClass) return nullptr;

	return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(!WeaponToEquip) return;

	//Remove weapon collisions
	WeaponToEquip->GetAreaSphere()->SetCollisionResponseToChannels(ECR_Ignore);
	WeaponToEquip->GetCollisionBox()->SetCollisionResponseToChannels(ECR_Ignore);
	
	if(const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket")))
	{
		HandSocket->AttachActor(WeaponToEquip, GetMesh());
	}
	EquippedWeapon = WeaponToEquip;
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(
		CrosshairShootTimer,
		this,
		&AShooterCharacter::FinishCrosshairBulletFire,
		ShootTimeDuration);
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(float const Value)
{
	const float TurnScaleFactor = bIsAiming ? MouseAimingTurnRate : MouseHipTurnRate;
	AddControllerYawInput(Value * TurnScaleFactor);
}

void AShooterCharacter::LookUp(float const Value)
{
	const float LookUpScaleFactor = bIsAiming ? MouseAimingLookUpRate : MouseHipLookUpRate;
	AddControllerPitchInput(Value * LookUpScaleFactor);
}
#pragma endregion

// BIND INPUT WITH ACTIONS
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	//Bind functions to action mappings (deprecated way)
	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);

	//Keyboard/Gamepad turns
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);

	//Mouse turns
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	//Actions
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);
}
