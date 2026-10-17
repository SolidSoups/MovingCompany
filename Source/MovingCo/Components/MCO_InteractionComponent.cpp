#include "MCO_InteractionComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/HitResult.h"
#include "GameFramework/Actor.h"
#include "MCO_InteractableComponent.h"
#include "MovingCo.h"
#include "Subsystems/MCO_InteractablesSubsystem.h"

UMCO_InteractionComponent::UMCO_InteractionComponent()
{
#if MCO_DEBUG
    PrimaryComponentTick.bCanEverTick = true;
#else
    PrimaryComponentTick.bCanEverTick = false;
#endif
}
void UMCO_InteractionComponent::BeginPlay()
{
    Super::BeginPlay();

    // Overlap only happens on server
    if (!GetOwner()->HasAuthority())
        return;

#if MCO_DEBUG
    if (bDebug)
    {
        SetComponentTickEnabled(true);
    }
    else
    {
        SetComponentTickEnabled(false);
    }
#endif
}

void UMCO_InteractionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if MCO_DEBUG
    if (bDebug)
    {
        if (UMCO_InteractablesSubsystem* InteractablesSS = GetWorld()->GetSubsystem<UMCO_InteractablesSubsystem>())
        {
            GEngine->AddOnScreenDebugMessage(
                19191922, 2.f, FColor::Yellow,
                FString::Printf(TEXT("Interactables: %d"), InteractablesSS->GetInteractables().Num()));

            // Draw dot product
            APawn* MyPawn = Cast<APawn>(GetOwner());
            if (!MyPawn)
                return;

            APlayerController* PC = Cast<APlayerController>(MyPawn->GetController());
            if (!PC)
                return;

            // Get camera transform
            FVector CameraLocation, CameraDirection;
            if (PC and PC->PlayerCameraManager)
            {
                CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
                CameraDirection = PC->PlayerCameraManager->GetCameraRotation().Vector();
            }
            else
                return;

            auto* Target = FindBestInteractable(CameraLocation, CameraDirection);
            if (!Target)
                return;

            // Draw range
            DrawDebugSphere(GetWorld(), Target->GetComponentLocation(), Target->GetInteractionRadius(), 32,
                            FColor::Green, false, 0.f, 0U, 0.5f);

            const FVector View = (Target->GetComponentLocation() - CameraLocation);
            const float ViewMag = View.Size();
            const float AngleRad = FMath::Acos(MinimumInteractionAlignement);
            const float Radius = FMath::Tan(AngleRad) * ViewMag;

            FVector Right = FVector::CrossProduct(FVector(0, 0, 1), CameraDirection).GetSafeNormal();
            FVector Up = FVector::CrossProduct(CameraDirection, Right).GetSafeNormal();

            DrawDebugCircle(GetWorld(), Target->GetComponentLocation(), Radius, 32, FColor::Yellow, false, -1.f, 0U,
                            0.0f, Right, Up, false);
        }
    }
#endif
}

void UMCO_InteractionComponent::Server_DoInteraction_Implementation()
{
    APawn* MyPawn = Cast<APawn>(GetOwner());
    if (!MyPawn)
        return;

    APlayerController* PC = Cast<APlayerController>(MyPawn->GetController());
    if (!PC)
        return;

    // Get camera transform
    FVector CameraLocation, CameraDirection;
    if (PC and PC->PlayerCameraManager)
    {
        CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
        CameraDirection = PC->PlayerCameraManager->GetCameraRotation().Vector();
    }
    else
    {
        return;
    }

    // Find the interactable that we think the player wants to interact with
    UMCO_InteractableComponent* Target = FindBestInteractable(CameraLocation, CameraDirection);
    if (!Target)
        return;

    Target->InteractWith(Cast<APawn>(GetOwner()));
}

UMCO_InteractableComponent* UMCO_InteractionComponent::FindBestInteractable(const FVector CameraLocation,
                                                                            const FVector CameraDirection) const
{
    UMCO_InteractablesSubsystem* InteractablesSS = GetWorld()->GetSubsystem<UMCO_InteractablesSubsystem>();
    if (!InteractablesSS)
        return nullptr;
    const auto& Interactables = InteractablesSS->GetInteractables();

    UMCO_InteractableComponent* BestInteractable = nullptr;
    float BestAlignment = -1.f;
    for (auto* Interactable : Interactables)
    {
        if (!IsValid(Interactable))
            continue;

        const FVector CamToInteractable = (Interactable->GetComponentLocation() - CameraLocation);

        // Make sure the interactable is not far away
        const float InteractionRadius = Interactable->GetInteractionRadius();
        const float DistSq = CamToInteractable.SizeSquared();
        if (DistSq >= InteractionRadius * InteractionRadius)
            continue;

        // Make sure the player is actually looking at it
        const float Alignment = FVector::DotProduct(CameraDirection.GetSafeNormal(), CamToInteractable.GetSafeNormal());
        if (Alignment < MinimumInteractionAlignement)
            continue;

        if (Alignment > BestAlignment)
        {
            BestInteractable = Interactable;
            BestAlignment = Alignment;
        }
    }

    return BestInteractable;
}
