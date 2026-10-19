#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MCO_WatcherAIController.generated.h"

class UMCO_VisionConeComponent;

UENUM(BlueprintType)
enum class EMCO_WatcherState : uint8
{
    /** Walking the patrol route (or wandering if there is none) */
    Patrol,
    /** Heard something, going to have a look */
    Investigate,
    /** Found a mover, walking up to them and staring */
    Confront
};

/**
 *  Stealth-game style AI for anything that watches the movers: the customer, and cows that panicked.
 *  Needs a UMCO_VisionConeComponent on the pawn and a nav mesh in the level.
 */
UCLASS()
class MOVINGCO_API AMCO_WatcherAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMCO_WatcherAIController();
    virtual void Tick(float DeltaSeconds) override;

    /** Called by the stealth subsystem whenever furniture bumps into something */
    void HandleBump(AActor* Source, const FVector& Location, float Loudness, APawn* Offender);

    UFUNCTION(BlueprintPure, Category="MCO|Watcher")
    EMCO_WatcherState GetWatcherState() const { return State; }

protected:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** Bumps further away than this (scaled by loudness) go unheard */
    UPROPERTY(EditAnywhere, Category="MCO|Watcher|Senses")
    float HearingRange = 2200.f;

    /** If true, any mover in sight is confronted, not just ones spotted while alert */
    UPROPERTY(EditAnywhere, Category="MCO|Watcher|Senses")
    bool bHuntPlayers = false;

    /** Stress given to a mover caught bumping furniture in plain sight */
    UPROPERTY(EditAnywhere, Category="MCO|Watcher|Stress")
    float CaughtStress = 35.f;

    /** Stress per second for every mover in sight while confronting someone */
    UPROPERTY(EditAnywhere, Category="MCO|Watcher|Stress")
    float GlareStressPerSecond = 6.f;

    UPROPERTY(EditAnywhere, Category="MCO|Watcher|Movement")
    float PatrolSpeed = 180.f;

    UPROPERTY(EditAnywhere, Category="MCO|Watcher|Movement")
    float AlertSpeed = 350.f;

    /** Radius used to pick random points when the pawn has no patrol route */
    UPROPERTY(EditAnywhere, Category="MCO|Watcher|Movement")
    float WanderRadius = 1500.f;

    UPROPERTY(EditAnywhere, Category="MCO|Watcher|Timing")
    float PatrolWaitTime = 2.5f;

    UPROPERTY(EditAnywhere, Category="MCO|Watcher|Timing")
    float InvestigateLookAroundTime = 4.f;

    /** How long to stare at a mover once standing in front of them */
    UPROPERTY(EditAnywhere, Category="MCO|Watcher|Timing")
    float ConfrontTime = 3.f;

    UPROPERTY(EditAnywhere, Category="MCO|Watcher|Timing")
    float ConfrontDistance = 200.f;

    /** Out of sight for this long during a confront and we go check their last known location */
    UPROPERTY(EditAnywhere, Category="MCO|Watcher|Timing")
    float LoseTargetTime = 2.f;

private:
    void SetState(EMCO_WatcherState NewState);
    void StartInvestigate(const FVector& Location);
    void StartConfront(APawn* Target);
    void ReturnToPatrol();

    void TickPatrol(float DeltaSeconds);
    void TickInvestigate(float DeltaSeconds);
    void TickConfront(float DeltaSeconds);
    void TickLookAround(float DeltaSeconds);

    void MoveToPatrolPoint(bool bAdvance);
    void ApplyGlare(float DeltaSeconds);
    APawn* FindVisiblePlayer() const;
    bool IsMoving() const;
    void SetMoveSpeed(float Speed) const;

    static void AddStressTo(APawn* Target, float Amount);

    EMCO_WatcherState State = EMCO_WatcherState::Patrol;
    TWeakObjectPtr<UMCO_VisionConeComponent> VisionCone;
    TWeakObjectPtr<APawn> ConfrontTarget;

    FVector InvestigateLocation = FVector::ZeroVector;
    FVector LastKnownTargetLocation = FVector::ZeroVector;
    int32 PatrolIndex = INDEX_NONE;
    bool bWaiting = false;
    float WaitRemaining = 0.f;
    float LookAroundRemaining = 0.f;
    float TimeTargetUnseen = 0.f;

    // The movement settings the pawn had before we took over, restored on unpossess
    bool bSavedUseControllerRotationYaw = true;
    bool bSavedUseControllerDesiredRotation = false;
    bool bSavedOrientRotationToMovement = false;
    float SavedMaxWalkSpeed = 0.f;
};

/** Takes over a cow whose stress maxed out. Wanders around hunting the other movers */
UCLASS()
class MOVINGCO_API AMCO_PanickedCowAIController : public AMCO_WatcherAIController
{
    GENERATED_BODY()

public:
    AMCO_PanickedCowAIController();
};
