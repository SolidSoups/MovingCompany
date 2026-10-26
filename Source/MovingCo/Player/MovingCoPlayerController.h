// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MovingCoPlayerController.generated.h"

class UMCO_PlayerHUDBase;
class UMCO_MainMenuWidget;
class AMCO_HUDBase;
class UInputMappingContext;
class UUserWidget;
class UInputAction;

/** Input modes applied through the CommonUI action router */
UENUM(BlueprintType)
enum class EMCO_InputMode : uint8
{
	Game,
	UI,
	GameAndUI
};

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

	/** Console command: create a Steam lobby and open the test map as a listen server */
	UFUNCTION(Exec)
	void HostGame();

	/** Console command: find the first MovingCo lobby and join it */
	UFUNCTION(Exec)
	void JoinGame();

	/** Sets the input mode on the CommonUI action router, which owns input routing (plain SetInputMode is ignored by it) */
	UFUNCTION(BlueprintCallable, Category="Input")
	void SetMCOInputMode(EMCO_InputMode Mode);

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

	/** Main menu widget class*/
	UPROPERTY(EditAnywhere, Category="UI|MainMenu")
	TSubclassOf<UMCO_MainMenuWidget> MainMenuWidgetClass;
	
	UPROPERTY(EditAnywhere, Category="UI|HUD")
	TSubclassOf<UMCO_PlayerHUDBase> PlayerHUDClass;
	
	UPROPERTY(BlueprintReadOnly, Category="UI|HUD")
	TObjectPtr<AMCO_HUDBase> HUDBase;
	
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

	/** Returns the HUD as an AMCO_HUDBase, caching it once it exists */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AMCO_HUDBase* GetHUDBase();

    // /** Traces from the camera and applies an impact to any destructible it hits */
    // void DoPush();
    //
    // /** Reset the destructible actor to the original state */
    // void DoReset();
};
