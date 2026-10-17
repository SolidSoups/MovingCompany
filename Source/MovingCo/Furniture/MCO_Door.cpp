#include "MCO_Door.h"
#include "Components/MCO_InteractableComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

AMCO_Door::AMCO_Door()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    SceneRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
    SetRootComponent(SceneRootComp);

    DoorMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door Static Mesh"));
    DoorMeshComp->SetMobility(EComponentMobility::Movable);
    DoorMeshComp->SetupAttachment(SceneRootComp);

    InteractableComponent = CreateDefaultSubobject<UMCO_InteractableComponent>(TEXT("Interactable Scene Component"));
    InteractableComponent->SetupAttachment(DoorMeshComp);
}

void AMCO_Door::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        InteractableComponent->OnInteractedWith.AddUniqueDynamic(this, &AMCO_Door::HandleOnInteractedWith);
    }
}

void AMCO_Door::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMCO_Door, MoveState);
}

void AMCO_Door::HandleOnInteractedWith(APawn* InstigatorPawn)
{
    if (!HasAuthority())
        return;

    const double Now = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

    MoveState.bOpen = !MoveState.bOpen;
    MoveState.StartTime = Now;
    OnRep_MoveState();
}

void AMCO_Door::OnRep_MoveState()
{
    if (MoveState.bOpen)
        OnOpenDoor(MoveState);
    else
        OnCloseDoor(MoveState);
}

