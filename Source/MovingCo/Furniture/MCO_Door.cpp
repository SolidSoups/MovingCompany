#include "MCO_Door.h"
#include "Components/MCO_InteractableComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"
#include "Curves/CurveFloat.h"

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

    if (bMoving)
        return;

    const double Now = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
    if(Now - MoveState.StartTime < MoveDuration)
        return;

    MoveState.bOpen = !MoveState.bOpen;
    MoveState.StartTime = Now;
}

void AMCO_Door::EndPlay(EEndPlayReason::Type EndPlayReason)
{
    if (MovingTickHandle.IsValid())
        FTSTicker::GetCoreTicker().RemoveTicker(MovingTickHandle);

    Super::EndPlay(EndPlayReason);
}

void AMCO_Door::Tick(float DeltaTime)
{
    const AGameStateBase* GS = GetWorld()->GetGameState();
    if(!GS)
        return;

    const double Elapsed = GS->GetServerWorldTimeSeconds() - MoveState.StartTime;
    const double T = FMath::Clamp(Elapsed / MoveDuration, 0.0, 1.0f);

    const UCurveFloat* Curve = MoveState.bOpen ? OpenCurve : CloseCurve;
    const float Progress = Curve ? Curve->GetFloatValue(T) : T;
    const float Yaw = OpeningAngle * (MoveState.bOpen ? Progress : 1.f - Progress);

    StaticMeshComp->SetRelativeRotation(FRotator(0.0f, StartingYaw + Yaw, 0.0f));

    if(MoveState.StartTime != LastSeenStartTime){
        LastSeenStartTime = MoveState.StartTime;
        bFXTriggered = false;
    }

    if(!bFXTriggered){
        if(MoveState.bOpen and Progress > 0.0f){
            OnDoorOpened();
            bFXTriggered = true;
        }
        else if (!MoveState.bOpen and Progress >= 0.95f){
            OnDoorClosed();
            bFXTriggered = true;
        }
    }

#if MCO_DEBUG
    if (bDebug)
    {
        GEngine->AddOnScreenDebugMessage(121912, 2.f, FColor::Green, 
                                         FString::Printf(TEXT("Door Progress: %.2f, Door Yaw: %.2f°"), Progress, Yaw));
    }
#endif
}

void AMCO_Door::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMCO_Door, DoorYawTarget);
}
