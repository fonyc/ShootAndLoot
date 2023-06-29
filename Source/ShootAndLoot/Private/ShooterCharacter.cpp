// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
{
    BaseTurnRate = 45.f;
    BaseLookUpRate = 45.f;
    
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent); //Camera follows the player at this distance behind the controller
    CameraBoom->TargetArmLength = 300.f; //Rotate the arm based on the controller
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); //Attach camera to end of CameraBoom
    FollowCamera->bUsePawnControlRotation = false; // Stop camera from rotate relative to the arm
    
    //Ensures we dont use the controller rotation for the character mesh
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false; 
    bUseControllerRotationRoll = false;

    //Configure Character Movement
    GetCharacterMovement()->bOrientRotationToMovement = true; //Character moves in the direction of input
    GetCharacterMovement()->RotationRate = FRotator(0.f,540.f,0.f);
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->AirControl = .2f;
    
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void AShooterCharacter::MoveForward(const float Value)
{
    if(Controller != nullptr && Value != 0)
    {
        //Find out which way is Forward
        const FRotator Rotation{Controller->GetControlRotation()};
        const FRotator YawRotation{0,Rotation.Yaw,0};

        const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X)};
        AddMovementInput(Direction, Value);
    }
}

void AShooterCharacter::MoveRight(const float Value)
{
    if(Controller != nullptr && Value != 0)
    {
        //Find out which way is Right
        const FRotator Rotation{Controller->GetControlRotation()};
        const FRotator YawRotation{0,Rotation.Yaw,0};

        const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y)};
        AddMovementInput(Direction, Value);
    }
}

void AShooterCharacter::FireWeapon()
{
    UE_LOG(LogTemp, Warning, TEXT("Weapon Fired!"))
}

void AShooterCharacter::TurnAtRate(float Rate)
{
    AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
    AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}


// Called to bind functionality to input
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
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

    //Actions
    PlayerInputComponent->BindAction("Jump", IE_Pressed,this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released,this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction("FireButton", IE_Pressed,this, &AShooterCharacter::FireWeapon);
}

