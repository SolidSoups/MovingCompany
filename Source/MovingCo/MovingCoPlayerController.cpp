// Copyright Epic Games, Inc. All Rights Reserved.


#include "MovingCoPlayerController.h"
#include "EnhancedInputComponent.h"
#include "Engine/World.h"
#include "MCO_DestructibleActor.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "MovingCoCameraManager.h"
#include "Blueprint/UserWidget.h"
#include "MovingCo.h"
#include "EngineUtils.h"
#include "Widgets/Input/SVirtualJoystick.h"

AMovingCoPlayerController::AMovingCoPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AMovingCoCameraManager::StaticClass();
}

void AMovingCoPlayerController::BeginPlay() {
  Super::BeginPlay();

  // only spawn touch controls on local player controllers
  if (IsLocalPlayerController() && ShouldUseTouchControls()) {
    // spawn the mobile controls widget
    MobileControlsWidget =
        CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

    if (MobileControlsWidget) {
      // add the controls to the player screen
      MobileControlsWidget->AddToPlayerScreen(0);

    } else {

      UE_LOG(LogMovingCo, Error,
             TEXT("Could not spawn mobile controls widget."));
    }
  }
}

void AMovingCoPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}

            if(UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent)){
                // Used for the Destructible wall test
                // EnhancedInputComponent->BindAction(PushAction, ETriggerEvent::Started, this, &AMovingCoPlayerController::DoPush);
                // EnhancedInputComponent->BindAction(ResetAction, ETriggerEvent::Started, this, &AMovingCoPlayerController::DoReset);
            }
		}
	}
}

bool AMovingCoPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

// Prototyping graveyard. 

// void AMovingCoPlayerController::DoPush(){
//     FVector ViewLocation;
//     FRotator ViewRotation;
//     GetPlayerViewPoint(ViewLocation, ViewRotation);
//
//     const FVector Direction = ViewRotation.Vector();
//     const FVector TraceEnd = ViewLocation + Direction * PushRange;
//
//     // Ignore our own pawn. The camera sits inside its capsule
//     const FCollisionQueryParams Params(SCENE_QUERY_STAT(PushTrace), false, GetPawn());
//
//     FHitResult Hit;
//     bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, ViewLocation, TraceEnd, ECC_Visibility, Params);
//
//     if(bHit){
//         DrawDebugLine(GetWorld(), ViewLocation + FVector(0.0, 0.0, -100.0), Hit.Location, FColor::Green, false, 1.f, 0U, 1.f);
//     }
//     else{
//         return;
//     }
//
//     if(AMCO_DestructibleActor* Structure = Cast<AMCO_DestructibleActor>(Hit.GetActor())){
//         Structure->ApplyRadialImpact(Hit.ImpactPoint, PushRadius, Direction, PushStrength);
//     }
// }
// void AMovingCoPlayerController::DoReset() {
//     for(TActorIterator<AMCO_DestructibleActor> It(GetWorld()); It; ++It){
//         It->Reset();
//     }
// }
