#include "MCO_WatcherAIController.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "MCO_CustomerCharacter.h"
#include "MCO_StealthSubsystem.h"
#include "MCO_VisionConeComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Player/MCO_StressComponent.h"

AMCO_WatcherAIController::AMCO_WatcherAIController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AMCO_WatcherAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    VisionCone = InPawn->FindComponentByClass<UMCO_VisionConeComponent>();
    if (!VisionCone.IsValid())
        UE_LOG(LogTemp, Warning, TEXT("'%s' has no vision cone, it will only be able to hear"), *GetNameSafe(InPawn));

    // Turn smoothly towards wherever we are focusing instead of snapping to it
    if (ACharacter* ControlledCharacter = Cast<ACharacter>(InPawn))
    {
        UCharacterMovementComponent* Movement = ControlledCharacter->GetCharacterMovement();
        bSavedUseControllerRotationYaw = ControlledCharacter->bUseControllerRotationYaw;
        bSavedUseControllerDesiredRotation = Movement->bUseControllerDesiredRotation;
        bSavedOrientRotationToMovement = Movement->bOrientRotationToMovement;
        SavedMaxWalkSpeed = Movement->MaxWalkSpeed;

        ControlledCharacter->bUseControllerRotationYaw = false;
        Movement->bUseControllerDesiredRotation = true;
        Movement->bOrientRotationToMovement = false;
    }

    if (UMCO_StealthSubsystem* Stealth = GetWorld()->GetSubsystem<UMCO_StealthSubsystem>())
        Stealth->RegisterWatcher(this);

    PatrolIndex = INDEX_NONE;
    SetState(EMCO_WatcherState::Patrol);
}

void AMCO_WatcherAIController::OnUnPossess()
{
    if (UMCO_StealthSubsystem* Stealth = GetWorld()->GetSubsystem<UMCO_StealthSubsystem>())
        Stealth->UnregisterWatcher(this);

    StopMovement();
    ClearFocus(EAIFocusPriority::Gameplay);

    if (ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn()))
    {
        UCharacterMovementComponent* Movement = ControlledCharacter->GetCharacterMovement();
        ControlledCharacter->bUseControllerRotationYaw = bSavedUseControllerRotationYaw;
        Movement->bUseControllerDesiredRotation = bSavedUseControllerDesiredRotation;
        Movement->bOrientRotationToMovement = bSavedOrientRotationToMovement;
        Movement->MaxWalkSpeed = SavedMaxWalkSpeed;
    }

    if (VisionCone.IsValid())
        VisionCone->SetAlertLevel(EMCO_AlertLevel::Calm);
    VisionCone = nullptr;

    Super::OnUnPossess();
}

void AMCO_WatcherAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UMCO_StealthSubsystem* Stealth = GetWorld()->GetSubsystem<UMCO_StealthSubsystem>())
        Stealth->UnregisterWatcher(this);

    Super::EndPlay(EndPlayReason);
}

void AMCO_WatcherAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!GetPawn())
        return;

    switch (State)
    {
    case EMCO_WatcherState::Patrol:
        TickPatrol(DeltaSeconds);
        break;
    case EMCO_WatcherState::Investigate:
        TickInvestigate(DeltaSeconds);
        break;
    case EMCO_WatcherState::Confront:
        TickConfront(DeltaSeconds);
        break;
    }
}

void AMCO_WatcherAIController::HandleBump(AActor* Source, const FVector& Location, float Loudness, APawn* Offender)
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn or Offender == MyPawn)
        return;

    bool bSawIt = false;
    if (VisionCone.IsValid())
    {
        const TArray<const AActor*> Ignored = {Source, Offender};
        bSawIt = VisionCone->CanSeePoint(Location, Ignored) or
                 (Source and VisionCone->CanSeePoint(Source->GetActorLocation(), Ignored));
    }

    if (bSawIt)
    {
        if (Offender and Offender->IsPlayerControlled())
        {
            AddStressTo(Offender, CaughtStress);
            StartConfront(Offender);
        }
        else
        {
            // Saw something fall over on its own, still worth a closer look
            StartInvestigate(Location);
        }
        return;
    }

    // Busy staring someone down, a noise elsewhere won't distract us
    if (State == EMCO_WatcherState::Confront)
        return;

    if (FVector::Dist(MyPawn->GetActorLocation(), Location) <= HearingRange * Loudness)
        StartInvestigate(Location);
}

void AMCO_WatcherAIController::SetState(EMCO_WatcherState NewState)
{
    State = NewState;
    bWaiting = false;
    LookAroundRemaining = 0.f;
    ClearFocus(EAIFocusPriority::Gameplay);

    SetMoveSpeed(State == EMCO_WatcherState::Patrol ? PatrolSpeed : AlertSpeed);

    if (VisionCone.IsValid())
    {
        static const EMCO_AlertLevel Levels[] = {EMCO_AlertLevel::Calm, EMCO_AlertLevel::Suspicious,
                                                 EMCO_AlertLevel::Alerted};
        VisionCone->SetAlertLevel(Levels[static_cast<uint8>(State)]);
    }
}

void AMCO_WatcherAIController::StartInvestigate(const FVector& Location)
{
    SetState(EMCO_WatcherState::Investigate);
    InvestigateLocation = Location;
    MoveToLocation(InvestigateLocation, 120.f, true, true, true, false);
}

void AMCO_WatcherAIController::StartConfront(APawn* Target)
{
    if (State == EMCO_WatcherState::Confront and ConfrontTarget == Target)
    {
        // Caught them again, restart the stare
        WaitRemaining = ConfrontTime;
        return;
    }

    SetState(EMCO_WatcherState::Confront);
    ConfrontTarget = Target;
    LastKnownTargetLocation = Target->GetActorLocation();
    TimeTargetUnseen = 0.f;
    WaitRemaining = ConfrontTime;
    SetFocus(Target, EAIFocusPriority::Gameplay);
}

void AMCO_WatcherAIController::ReturnToPatrol()
{
    ConfrontTarget = nullptr;
    SetState(EMCO_WatcherState::Patrol);
    MoveToPatrolPoint(false);
}

void AMCO_WatcherAIController::TickPatrol(float DeltaSeconds)
{
    if (bHuntPlayers)
    {
        if (APawn* Seen = FindVisiblePlayer())
        {
            StartConfront(Seen);
            return;
        }
    }

    if (IsMoving())
        return;

    if (!bWaiting)
    {
        bWaiting = true;
        WaitRemaining = PatrolWaitTime;
    }

    TickLookAround(DeltaSeconds);

    WaitRemaining -= DeltaSeconds;
    if (WaitRemaining <= 0.f)
    {
        bWaiting = false;
        ClearFocus(EAIFocusPriority::Gameplay);
        MoveToPatrolPoint(true);
    }
}

void AMCO_WatcherAIController::TickInvestigate(float DeltaSeconds)
{
    if (APawn* Seen = FindVisiblePlayer())
    {
        StartConfront(Seen);
        return;
    }

    if (IsMoving())
        return;

    if (!bWaiting)
    {
        bWaiting = true;
        WaitRemaining = InvestigateLookAroundTime;
    }

    TickLookAround(DeltaSeconds);

    WaitRemaining -= DeltaSeconds;
    if (WaitRemaining <= 0.f)
        ReturnToPatrol();
}

void AMCO_WatcherAIController::TickConfront(float DeltaSeconds)
{
    APawn* Target = ConfrontTarget.Get();
    if (!Target or !Target->IsPlayerControlled())
    {
        ReturnToPatrol();
        return;
    }

    ApplyGlare(DeltaSeconds);

    if (VisionCone.IsValid() and VisionCone->CanSeeActor(Target))
    {
        TimeTargetUnseen = 0.f;
        LastKnownTargetLocation = Target->GetActorLocation();
    }
    else
    {
        TimeTargetUnseen += DeltaSeconds;
        if (TimeTargetUnseen > LoseTargetTime)
        {
            StartInvestigate(LastKnownTargetLocation);
            return;
        }
    }

    const float Distance = FVector::Dist2D(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    if (Distance > ConfrontDistance + 75.f)
    {
        if (!IsMoving())
            MoveToActor(Target, ConfrontDistance, true, true, false);
        return;
    }

    // Standing right in front of them, staring
    WaitRemaining -= DeltaSeconds;
    if (WaitRemaining <= 0.f)
        ReturnToPatrol();
}

void AMCO_WatcherAIController::TickLookAround(float DeltaSeconds)
{
    LookAroundRemaining -= DeltaSeconds;
    if (LookAroundRemaining > 0.f)
        return;

    LookAroundRemaining = FMath::FRandRange(0.8f, 1.6f);

    const FVector Origin = GetPawn()->GetActorLocation();
    const float Yaw = GetPawn()->GetActorRotation().Yaw + FMath::FRandRange(-110.f, 110.f);
    SetFocalPoint(Origin + FRotator(0.f, Yaw, 0.f).Vector() * 500.f, EAIFocusPriority::Gameplay);
}

void AMCO_WatcherAIController::MoveToPatrolPoint(bool bAdvance)
{
    const AMCO_CustomerCharacter* Customer = Cast<AMCO_CustomerCharacter>(GetPawn());
    const int32 NumPoints = Customer ? Customer->GetNumPatrolPoints() : 0;

    if (NumPoints > 0)
    {
        if (bAdvance or PatrolIndex == INDEX_NONE)
            PatrolIndex = (PatrolIndex + 1) % NumPoints;

        MoveToLocation(Customer->GetPatrolPoint(PatrolIndex), 50.f, true, true, true, false);
        return;
    }

    // No route, wander around
    if (!bAdvance)
        return;

    if (UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
    {
        FNavLocation Destination;
        if (Nav->GetRandomReachablePointInRadius(GetPawn()->GetActorLocation(), WanderRadius, Destination))
            MoveToLocation(Destination.Location, 50.f, true, true, false, false);
    }
}

void AMCO_WatcherAIController::ApplyGlare(float DeltaSeconds)
{
    if (GlareStressPerSecond <= 0.f or !VisionCone.IsValid())
        return;

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APawn* PlayerPawn = It->Get() ? It->Get()->GetPawn() : nullptr;
        if (PlayerPawn and PlayerPawn != GetPawn() and VisionCone->CanSeeActor(PlayerPawn))
            AddStressTo(PlayerPawn, GlareStressPerSecond * DeltaSeconds);
    }
}

APawn* AMCO_WatcherAIController::FindVisiblePlayer() const
{
    if (!VisionCone.IsValid())
        return nullptr;

    APawn* Closest = nullptr;
    float ClosestDistSq = TNumericLimits<float>::Max();

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APawn* PlayerPawn = It->Get() ? It->Get()->GetPawn() : nullptr;
        if (!PlayerPawn or PlayerPawn == GetPawn() or !VisionCone->CanSeeActor(PlayerPawn))
            continue;

        const float DistSq = FVector::DistSquared(PlayerPawn->GetActorLocation(), GetPawn()->GetActorLocation());
        if (DistSq < ClosestDistSq)
        {
            ClosestDistSq = DistSq;
            Closest = PlayerPawn;
        }
    }
    return Closest;
}

bool AMCO_WatcherAIController::IsMoving() const
{
    return GetMoveStatus() != EPathFollowingStatus::Idle;
}

void AMCO_WatcherAIController::SetMoveSpeed(float Speed) const
{
    if (const ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn()))
        ControlledCharacter->GetCharacterMovement()->MaxWalkSpeed = Speed;
}

void AMCO_WatcherAIController::AddStressTo(APawn* Target, float Amount)
{
    if (UMCO_StressComponent* Stress = Target ? Target->FindComponentByClass<UMCO_StressComponent>() : nullptr)
        Stress->AddStress(Amount);
}

AMCO_PanickedCowAIController::AMCO_PanickedCowAIController()
{
    bHuntPlayers = true;
    HearingRange = 1500.f;

    // Just follows people around, it only gets mad when they break stuff in front of it
    GlareStressPerSecond = 0.f;
    CaughtStress = 25.f;

    PatrolSpeed = 250.f;
    AlertSpeed = 380.f;
    PatrolWaitTime = 1.f;
    ConfrontTime = 5.f;
}
