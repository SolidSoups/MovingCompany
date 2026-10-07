// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "MovingCoCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UPhysicsHandleComponent;
struct FInputActionValue;

class UMCO_StaminaComponent;

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

protected:

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

    UFUNCTION(BlueprintCallable, Category="Input")
    void DoStartHold();

    UFUNCTION(BlueprintCallable, Category="Input")
    void DoEndHold();

    FVector GetHoldAnchor() const;
    bool bLifting = false;
    bool bHolding = false;
    FVector HoldOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category="Hold", meta=(ClampMin="1"))
    float HalfSpeedMass = 500.f;

    UPROPERTY(EditAnywhere, Category="Hold", meta=(ClampMin="0.1", ClampMax="1"))
    float DragSpeedMultiplier = 0.6f;

    float HoldSpeedScale = 1.0f;
    void SetHoldSpeedScale(float Scale);

    UPROPERTY(EditDefaultsOnly, Category="Stagger")
    float StaggerDuration = 0.8f;

    UFUNCTION()
    void HandleStaminaFullyDrained();

    bool bStaggered = false;
    FTimerHandle StaggerTimer;
    void EndStagger();

protected:
    virtual void BeginPlay() override;

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

