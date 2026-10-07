// Copyright Epic Games, Inc. All Rights Reserved.

#include "MovingCoCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "MCO_FurnitureBase.h"
#include "MCO_StaminaComponent.h"
#include "MovingCo.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

AMovingCoCharacter::AMovingCoCharacter()
{
    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

    // Create the first person mesh that will be viewed only by this character's
    // owner
    FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

    FirstPersonMesh->SetupAttachment(GetMesh());
    FirstPersonMesh->SetOnlyOwnerSee(true);
    FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
    FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

    // Create the Camera Component
    FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
    FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
    FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f),
                                                               FRotator(0.0f, 90.0f, -90.0f));
    FirstPersonCameraComponent->bUsePawnControlRotation = true;
    FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
    FirstPersonCameraComponent->bEnableFirstPersonScale = true;
    FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
    FirstPersonCameraComponent->FirstPersonScale = 0.6f;

    // configure the character comps
    GetMesh()->SetOwnerNoSee(true);
    GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

    GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

    // Configure character movement
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
    GetCharacterMovement()->AirControl = 0.5f;
    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

    PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("Physics Handle"));

    StaminaComponent = CreateDefaultSubobject<UMCO_StaminaComponent>(TEXT("Stamina Component"));
}
void AMovingCoCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    FirstPersonMesh->AddRelativeLocation(FVector(0.f, 0.f, -HalfHeightAdjust));
}
void AMovingCoCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    FirstPersonMesh->AddRelativeLocation(FVector(0.f, 0.f, HalfHeightAdjust));
}

void AMovingCoCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (PhysicsHandle->GetGrabbedComponent())
    {
        const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
        PhysicsHandle->SetTargetLocationAndRotation(GetHoldAnchor() + YawRotation.RotateVector(HoldOffset),
                                                    YawRotation);
    }
}

void AMovingCoCharacter::BeginPlay()
{
    Super::BeginPlay();

    StaminaComponent->OnStaminaFullyDrained.AddDynamic(this, &AMovingCoCharacter::HandleStaminaFullyDrained);
}

void AMovingCoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    // Set up action bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Jumping
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMovingCoCharacter::DoJumpStart);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMovingCoCharacter::DoJumpEnd);

        // Crouching
        EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this,
                                           &AMovingCoCharacter::DoStartCrouch);
        EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this,
                                           &AMovingCoCharacter::DoEndCrouch);

        // Moving
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMovingCoCharacter::MoveInput);

        // Looking/Aiming
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMovingCoCharacter::LookInput);
        EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this,
                                           &AMovingCoCharacter::LookInput);

        EnhancedInputComponent->BindAction(HoldAction, ETriggerEvent::Started, this, &AMovingCoCharacter::DoStartHold);
        EnhancedInputComponent->BindAction(HoldAction, ETriggerEvent::Completed, this, &AMovingCoCharacter::DoEndHold);
    }
    else
    {
        UE_LOG(LogMovingCo, Error,
               TEXT("'%s' Failed to find an Enhanced Input Component! This template "
                    "is built to use the Enhanced Input system. If you intend to use "
                    "the legacy system, then you will need to update this C++ file."),
               *GetNameSafe(this));
    }
}

void AMovingCoCharacter::MoveInput(const FInputActionValue& Value)
{
    // get the Vector2D move axis
    FVector2D MovementVector = Value.Get<FVector2D>();

    // pass the axis values to the move input
    DoMove(MovementVector.X, MovementVector.Y);
}

void AMovingCoCharacter::LookInput(const FInputActionValue& Value)
{
    // get the Vector2D look axis
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    // pass the axis values to the aim input
    DoAim(LookAxisVector.X, LookAxisVector.Y);
}

void AMovingCoCharacter::DoAim(float Yaw, float Pitch)
{
    if (GetController())
    {
        // pass the rotation inputs
        AddControllerYawInput(Yaw * HoldSpeedScale);
        AddControllerPitchInput(Pitch * HoldSpeedScale);
    }
}

void AMovingCoCharacter::DoMove(float Right, float Forward)
{
    if (bStaggered)
        return;

    if (GetController())
    {
        // pass the move inputs
        AddMovementInput(GetActorRightVector(), Right);
        AddMovementInput(GetActorForwardVector(), Forward);
    }
}

void AMovingCoCharacter::DoJumpStart()
{
    if(bHolding)
        return;

    // pass Jump to the character
    Jump();
}

void AMovingCoCharacter::DoJumpEnd()
{
    // pass StopJumping to the character
    StopJumping();
}

// Crouching
void AMovingCoCharacter::DoStartCrouch()
{
    Crouch();
}
void AMovingCoCharacter::DoEndCrouch()
{
    UnCrouch();
}

void AMovingCoCharacter::DoStartHold()
{
    if(bStaggered)
        return;

    // Trace to see if there is furniture in front of us
    const FVector TraceStart = FirstPersonCameraComponent->GetComponentLocation();
    const FVector TraceEnd = TraceStart + FirstPersonCameraComponent->GetForwardVector() * 250.f;

    const FCollisionQueryParams Params(SCENE_QUERY_STAT(HoldTrace), false, this);

    FHitResult Hit;
    if (!GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
    {
        return;
    }

    if (!Cast<AMCO_FurnitureBase>(Hit.GetActor()))
        return;

    bLifting = bIsCrouched;

    // Start draining stamina
    if (StaminaComponent)
        StaminaComponent->StartStaminaDrain(bLifting);

    // Grab furniture with physics handle
    const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
    HoldOffset = YawRotation.UnrotateVector(Hit.ImpactPoint - GetHoldAnchor());
    PhysicsHandle->GrabComponentAtLocationWithRotation(
        Hit.GetComponent(),
        NAME_None,
        Hit.ImpactPoint,
        YawRotation
    );

    // Adjust speed by mass of furniture
    float SpeedScale = HalfSpeedMass / (HalfSpeedMass + Hit.GetComponent()->GetMass());
    if(!bLifting)
        SpeedScale *= DragSpeedMultiplier;
    SetHoldSpeedScale(SpeedScale);

    bHolding = true;
}
void AMovingCoCharacter::DoEndHold()
{
    PhysicsHandle->ReleaseComponent();
    SetHoldSpeedScale(1.0f);

    if (StaminaComponent)
        StaminaComponent->EndStaminaDrain();

    bHolding = false;
}

FVector AMovingCoCharacter::GetHoldAnchor() const
{
    if (bLifting)
        return GetActorLocation();

    return GetActorLocation() - FVector(0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
}

void AMovingCoCharacter::SetHoldSpeedScale(float Scale)
{
    HoldSpeedScale = Scale;

    const UCharacterMovementComponent* DefaultMovement =
        GetDefault<AMovingCoCharacter>(GetClass())->GetCharacterMovement();
    GetCharacterMovement()->MaxWalkSpeed = DefaultMovement->MaxWalkSpeed * Scale;
    GetCharacterMovement()->MaxWalkSpeedCrouched = DefaultMovement->MaxWalkSpeedCrouched * Scale;
}
void AMovingCoCharacter::HandleStaminaFullyDrained()
{
    UPrimitiveComponent* Dropped = PhysicsHandle->GetGrabbedComponent();

    DoEndHold();

    if(Dropped){
        const FVector Fumble = 
            GetActorForwardVector() * 150.f + FVector(0.f, 0.f, -300.f);
        Dropped->AddImpulse(Fumble, NAME_None, true);
        Dropped->AddAngularImpulseInDegrees(FMath::VRand() * 180.f, NAME_None,
                                            true);
    }

    GetCharacterMovement()->StopMovementImmediately();

    bStaggered = true;
    GetWorldTimerManager().SetTimer(StaggerTimer, this, 
                                    &AMovingCoCharacter::EndStagger, StaggerDuration);
}

void AMovingCoCharacter::EndStagger(){
    bStaggered = false;
}
