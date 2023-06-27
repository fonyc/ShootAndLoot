// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent); //Camera follows the player at this distance behind the controller
    CameraBoom->TargetArmLength = 300.f; //Rotate the arm based on the controller
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); //Attach camera to end of CameraBoom
    FollowCamera->bUsePawnControlRotation = false; // Stop camera from rotate relative to the arm
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
    Super::BeginPlay();

     if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
     {
         if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
             ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
         {
             Subsystem->AddMappingContext(CharacterIMP, 0);
         }
     }
}

 void AShooterCharacter::Move(const FInputActionValue& Value)
 {
    if(const bool CurrentValue = Value.Get<bool>())
    {
        UE_LOG(LogTemp, Warning, TEXT("IA_Move Triggerd!"));
    }
 }

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if(UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered,this,&AShooterCharacter::Move);
    }
}

