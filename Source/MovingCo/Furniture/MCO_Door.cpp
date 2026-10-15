#include "MCO_Door.h"
#include "Components/StaticMeshComponent.h"
#include "Components/MCO_InteractableComponent.h"
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
    InteractableComponent->OnInteractedWith.AddUniqueDynamic(this, &AMCO_Door::HandleOnInteractedWith);
}

void AMCO_Door::HandleOnInteractedWith(APawn* InstigatorPawn)
{
    if(!HasAuthority())
        return;

    // Already moving, let's getoutahere
    if(bMoving)
        return;

    if(MovingTickHandle.IsValid())
        FTSTicker::GetCoreTicker().RemoveTicker(MovingTickHandle);


    DoorYaw = bOpen ? -90.0f : 0.f; // just to make sure that we start from the correct yaw
    OnRep_DoorYaw();
    FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &AMCO_Door::TickMovingDoor, !bOpen), 0.f);
}

void AMCO_Door::EndPlay(EEndPlayReason::Type EndPlayReason) {
    if(MovingTickHandle.IsValid())
        FTSTicker::GetCoreTicker().RemoveTicker(MovingTickHandle);

    Super::EndPlay(EndPlayReason);
}

void AMCO_Door::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMCO_Door, DoorYaw);
}

bool AMCO_Door::TickMovingDoor(float DeltaTime, bool bShouldOpen){
    float SpeedWithDir = OpenSpeed * (bShouldOpen ? -1.f : 1.f);
    DoorYaw += SpeedWithDir * DeltaTime; 
    OnRep_DoorYaw();

    if(bShouldOpen and DoorYaw <= -90.f or !bShouldOpen and DoorYaw >= 0.f){
        DoorYaw = bShouldOpen ? -90.f : 0.0f;
        bOpen = bShouldOpen;
        return false;
    }

    return true;
}

void AMCO_Door::OnRep_DoorYaw()
{
    FRotator NewRotation = FRotator(0.0f, DoorYaw, 0.0f);
    StaticMeshComp->SetRelativeRotation(NewRotation);
}
