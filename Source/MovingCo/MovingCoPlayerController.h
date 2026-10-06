// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MovingCoPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UInputAction;

/**
 *  Simple first person Player Controller
 *  Manages the input mapping context.
 *  Overrides the Player Camera Manager class.
 */
UCLASS(abstract, config="Game")
class MOVINGCO_API AMovingCoPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	AMovingCoPlayerController();

protected:
    UPROPERTY(EditAnywhere, Category="Push")
    float PushRange = 5000.0f;

    UPROPERTY(EditAnywhere, Category="Push")
    float PushStrength = 2000.0f;

    UPROPERTY(EditAnywhere, Category="Push")
    float PushRadius = 1000.f;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

    // UPROPERTY(EditAnywhere, Category="Input")
    // UInputAction* PushAction;
    //
    // UPROPERTY(EditAnywhere, Category="Input")
    // UInputAction* ResetAction;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

    // /** Traces from the camera and applies an impact to any destructible it hits */
    // void DoPush();
    //
    // /** Reset the destructible actor to the original state */
    // void DoReset();
};
