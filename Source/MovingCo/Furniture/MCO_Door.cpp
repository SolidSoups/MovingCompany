#include "MCO_Door.h"
#include "MovingCo.h"
#include "Components/MCO_InteractableComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Components/SceneComponent.h"
#include "UI/MCO_ObjectiveUserWidget.h"

const FName AMCO_Door::DoorSceneCompName(TEXT("Door Scene Component"));

AMCO_Door::AMCO_Door(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    SceneRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
    SetRootComponent(SceneRootComp);

    DoorSceneComp = CreateDefaultSubobject<USceneComponent, UStaticMeshComponent>(DoorSceneCompName);
    DoorSceneComp->SetMobility(EComponentMobility::Movable);
    DoorSceneComp->SetupAttachment(SceneRootComp);

    InteractableComponent = CreateDefaultSubobject<UMCO_InteractableComponent>(TEXT("Interactable Scene Component"));
    InteractableComponent->SetupAttachment(DoorSceneComp);

    ObjectiveText = NSLOCTEXT("MCO", "DefaultDoorObjective", "Make your way to the highlighted door.");
    ObjectiveWidgetClass = UMCO_ObjectiveUserWidget::StaticClass();
}

void AMCO_Door::OnConstruction(const FTransform& Transform)
{

}

void AMCO_Door::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        InteractableComponent->OnInteractedWith.AddUniqueDynamic(this, &AMCO_Door::HandleOnInteractedWith);
    }

    if (bIsObjective and GetNetMode() != NM_DedicatedServer)
    {
        if (HighlightMaterial)
            SetHighlighted(true);
        else
            UE_LOG(LogMovingCo, Warning, TEXT("%s is an objective door without a HighlightMaterial, it won't show through walls"), *GetName());
    }
}

void AMCO_Door::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ObjectiveWidget)
    {
        ObjectiveWidget->RemoveFromParent();
        ObjectiveWidget = nullptr;
    }
    Super::EndPlay(EndPlayReason);
}

void AMCO_Door::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bIsObjective and !bObjectiveCleared)
        UpdateObjective();
}

void AMCO_Door::UpdateObjective()
{
    // Purely cosmetic, so it's all done against the local player. Lazy since there might not be one yet in BeginPlay
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC or !PC->IsLocalController())
        return;

    if (!ObjectiveWidget and ObjectiveWidgetClass and !ObjectiveText.IsEmpty())
    {
        ObjectiveWidget = CreateWidget<UMCO_ObjectiveUserWidget>(PC, ObjectiveWidgetClass);
        if (ObjectiveWidget)
        {
            // Added first so the default layout exists before the text goes in
            ObjectiveWidget->AddToViewport();
            ObjectiveWidget->SetObjectiveText(ObjectiveText);
        }
    }

    if (bObjectiveReached)
        return;

    const APawn* Pawn = PC->GetPawn();
    const float Distance = Pawn ? FVector::Dist(Pawn->GetActorLocation(), DoorSceneComp->Bounds.Origin) : MAX_flt;
    if (Distance <= HighlightRevealDistance)
    {
        UE_LOG(LogMovingCo, Display, TEXT("%s reached by the local player (%.0f cm away), removing highlight"), *GetName(), Distance);
        bObjectiveReached = true;
        SetHighlighted(false);
        OnObjectiveReached();
    }
}

void AMCO_Door::SetHighlighted(bool bHighlighted)
{
    // Covers every mesh, including the frame and anything a Blueprint subclass adds
    TInlineComponentArray<UMeshComponent*> Meshes(this);
    for (UMeshComponent* Mesh : Meshes)
        Mesh->SetOverlayMaterial(bHighlighted ? HighlightMaterial.Get() : nullptr);

    UE_LOG(LogMovingCo, Display, TEXT("%s highlight %s on %d mesh(es)"), *GetName(), bHighlighted ? TEXT("on") : TEXT("off"), Meshes.Num());
}

void AMCO_Door::ClearObjective()
{
    bObjectiveCleared = true;
    bObjectiveReached = true;
    SetHighlighted(false);

    if (ObjectiveWidget)
    {
        ObjectiveWidget->RemoveFromParent();
        ObjectiveWidget = nullptr;
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

