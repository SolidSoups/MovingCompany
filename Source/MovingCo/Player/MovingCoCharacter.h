// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UI/MCO_CursorWidget.h"
#include "Logging/LogMacros.h"
#include "MovingCoCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UPhysicsHandleComponent;
struct FInputActionValue;

class UMCO_StaminaComponent;
class UMCO_StressComponent;
class UMCO_VisionConeComponent;
class AMCO_FurnitureBase;
class AAIController;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A basic first person character
 */
UCLASS(abstract)
class AMovingCoCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

    UPROPERTY(VisibleAnywhere, Category="Components", meta = (AllowPrivateAccess = "true"))
    UPhysicsHandleComponent* PhysicsHandle;

    UPROPERTY(VisibleAnywhere, Category="Components", meta = (AllowPrivateAccess = "true"))
    UMCO_StaminaComponent* StaminaComponent;

    UPROPERTY(VisibleAnywhere, Category="Components", meta = (AllowPrivateAccess = "true"))
    UMCO_StressComponent* StressComponent;

    /** Only shown while panicking, when this cow is an NPC looking for the other movers */
    UPROPERTY(VisibleAnywhere, Category="Components", meta = (AllowPrivateAccess = "true"))
    UMCO_VisionConeComponent* PanicVisionCone;

protected:
    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<class UMCO_CursorWidget> CursorWidgetClass;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* MouseLookAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* CrouchAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* HoldAction;
	
public:
	AMovingCoCharacter();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
    virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

    virtual void Tick(float DeltaSeconds) override;


protected:

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

    UFUNCTION(BlueprintCallable, Category="Input")
    void DoStartCrouch();

    UFUNCTION(BlueprintCallable, Category="Input")
    void DoEndCrouch();

    UFUNCTION(Server, Reliable)
    void Server_StartHolding();

    UFUNCTION(Server, Reliable)
    void Server_StopHolding();

    FVector GetHoldAnchor() const;
    bool bLifting = false;
    FVector HoldOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category="Hold", meta=(ClampMin="1"))
    float HalfSpeedMass = 500.f;

    UPROPERTY(EditAnywhere, Category="Hold", meta=(ClampMin="0.1", ClampMax="1"))
    float DragSpeedMultiplier = 0.6f;

    UPROPERTY(ReplicatedUsing=OnRep_HoldSpeedScale) 
    float HoldSpeedScale = 1.0f;

    UPROPERTY(Replicated) 
    bool bHolding = false;

    void SetHoldSpeedScale(float Scale);

    UPROPERTY(EditDefaultsOnly, Category="Stagger")
    float StaggerDuration = 0.8f;

    UFUNCTION()
    void HandleStaminaFullyDrained();

    bool bStaggered = false;
    FTimerHandle StaggerTimer;
    void EndStagger();

    /** AI that takes over the cow while it panics */
    UPROPERTY(EditDefaultsOnly, Category="Stress")
    TSubclassOf<AAIController> PanicControllerClass;

    UFUNCTION()
    void HandlePanicChanged(bool bPanicking);

    TWeakObjectPtr<AMCO_FurnitureBase> HeldFurniture;

    /** The player that lost control of us, gets it back once we calm down */
    TWeakObjectPtr<APlayerController> PanickedPlayer;

    UPROPERTY()
    TObjectPtr<AAIController> PanicController;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
    virtual void NotifyControllerChanged() override;

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

    UFUNCTION(BlueprintCallable, Category="UI") 
    UMCO_CursorWidget* GetCursorWidget() const { return CursorWidget; }

private:
    UFUNCTION()
    void OnRep_HoldSpeedScale() {
        SetHoldSpeedScale(HoldSpeedScale);
    }

    void RemoveCursorWidget();
    UPROPERTY()
    TObjectPtr<UMCO_CursorWidget> CursorWidget;
};

