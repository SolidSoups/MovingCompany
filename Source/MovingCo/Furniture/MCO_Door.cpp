#include "MCO_Door.h"
#include "Components/MCO_InteractableComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

AMCO_Door::AMCO_Door()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door Mesh"));
    StaticMeshComp->SetMobility(EComponentMobility::Movable);
    SetRootComponent(StaticMeshComp);

    InteractableComponent = CreateDefaultSubobject<UMCO_InteractableComponent>(TEXT("Interactable Scene Component"));
    InteractableComponent->SetupAttachment(StaticMeshComp);
}

void AMCO_Door::BeginPlay()
{
    Super::BeginPlay();

    StartingYaw = StaticMeshComp->GetComponentRotation().Yaw;

        if (HasAuthority())
    {
        InteractableComponent->OnInteractedWith.AddUniqueDynamic(this, &AMCO_Door::HandleOnInteractedWith);
    }
}

void AMCO_Door::HandleOnInteractedWith(APawn* InstigatorPawn)
{
    if (!HasAuthority())
        return;

    // Already moving, let's getoutahere
    if (bMoving)
        return;

    // Update door yaw
    DoorYawTarget = bOpen ? OpeningAngle : 0.f;

    if (MovingTickHandle.IsValid())
        FTSTicker::GetCoreTicker().RemoveTicker(MovingTickHandle);

    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &AMCO_Door::TickMovingDoor, !bOpen), 0.f);
}

void AMCO_Door::EndPlay(EEndPlayReason::Type EndPlayReason)
{
    if (MovingTickHandle.IsValid())
        FTSTicker::GetCoreTicker().RemoveTicker(MovingTickHandle);

    Super::EndPlay(EndPlayReason);
}

void AMCO_Door::Tick(float DeltaTime)
{
    float Alpha = 1.f - FMath::Exp(-SmoothingDecayRate * DeltaTime);
    CurrentDoorYaw = FMath::Lerp(CurrentDoorYaw, DoorYawTarget, Alpha);
    StaticMeshComp->SetRelativeRotation(FRotator(0.0f, StartingYaw + CurrentDoorYaw, 0.f));

    if (bDebug)
    {
        const float WireAngleRad = FMath::DegreesToRadians(DoorYawTarget);
        const float CurrentAngleRad = FMath::DegreesToRadians(CurrentDoorYaw);
        const float Radius = 200.f;

        FVector TargetDebugLocation;
        TargetDebugLocation.X = Radius * FMath::Cos(WireAngleRad);
        TargetDebugLocation.Y = Radius * FMath::Sin(WireAngleRad);
        TargetDebugLocation.Z = 0.f;

        FVector CurrentDebugLocation;
        CurrentDebugLocation.X = Radius * FMath::Cos(CurrentAngleRad);
        CurrentDebugLocation.Y = Radius * FMath::Sin(CurrentAngleRad);
        CurrentDebugLocation.Z = 0.f;

        DrawDebugSphere(GetWorld(), GetActorLocation() + TargetDebugLocation, 5.f, 8, FColor::Red);
        DrawDebugSphere(GetWorld(), GetActorLocation() + CurrentDebugLocation, 2.5f, 8, FColor::Green);
    }
}

void AMCO_Door::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMCO_Door, DoorYawTarget);
}

bool AMCO_Door::TickMovingDoor(float DeltaTime, bool bShouldOpen)
{
    float SpeedWithDir = OpenSpeed * (bShouldOpen ? -1.f : 1.f);
    DoorYawTarget += SpeedWithDir * DeltaTime;

    if (bShouldOpen and DoorYawTarget <= -90.f or !bShouldOpen and DoorYawTarget >= 0.f)
    {
        DoorYawTarget = bShouldOpen ? -90.f : 0.0f;
        bOpen = bShouldOpen;
        if (bShouldOpen)
            Multicast_DoorOpened();
        else
            Multicast_DoorClosed();

        return false;
    }

    return true;
}

void AMCO_Door::Multicast_DoorClosed_Implementation()
{
    OnDoorClosed();
}
void AMCO_Door::Multicast_DoorOpened_Implementation()
{
    OnDoorOpened();
}
