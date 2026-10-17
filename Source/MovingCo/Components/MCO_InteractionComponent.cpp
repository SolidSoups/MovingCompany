#include "MCO_InteractionComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/HitResult.h"
#include "GameFramework/Actor.h"
#include "MCO_InteractableComponent.h"
#include "MovingCo.h"

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

    DetectionSphere = NewObject<USphereComponent>(GetOwner(), TEXT("Detection Sphere Comp"));
    DetectionSphere->InitSphereRadius(200.f);
    DetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    DetectionSphere->SetupAttachment(GetOwner()->GetRootComponent());
    DetectionSphere->RegisterComponent();

    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &UMCO_InteractionComponent::OnBeginOverlap);
    DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &UMCO_InteractionComponent::OnEndOverlap);

#if MCO_DEBUG
    if(bDebug){
        SetComponentTickEnabled(true); 
    }
    else{
        SetComponentTickEnabled(false);
    }
#endif
}

void UMCO_InteractionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if MCO_DEBUG
    for(UMCO_InteractableComponent* Comp : Interactables){
        if(!IsValid(Comp))
            continue;
        // Draw center
        DrawDebugSphere(GetWorld(), Comp->GetComponentLocation(), 5.f,
                        32, FColor::Red, false, 0.f, 0U, 0.5f);
        // Draw range
        DrawDebugSphere(GetWorld(), Comp->GetComponentLocation(), Comp->GetInteractionRadius(),
                        32, FColor::Green, false, 0.f, 0U, 0.5f);
    } 
#endif
}

void UMCO_InteractionComponent::Server_DoInteraction_Implementation()
{
    APawn* MyPawn = Cast<APawn>(GetOwner());
    if(!MyPawn) return;

    APlayerController* PC = Cast<APlayerController>(MyPawn->GetController());
    if(!PC) return;

    // Get camera transform
    FVector CameraLocation, CameraDirection;
    if(PC and PC->PlayerCameraManager){
       CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
        CameraDirection = PC->PlayerCameraManager->GetCameraRotation().Vector();
    }
    else{
        return;
    }

#if MCO_DEBUG
    if(bDebug){
       DrawDebugLine(GetWorld(), CameraLocation, CameraLocation + CameraDirection * 500.f, FColor::Red,
                     false, 2.0f, 0U, 0.5f); 
    }
#endif

    UMCO_InteractableComponent* BestInteractable = nullptr;
    float BestAlignment = -1.f;
    for (auto* Interactable : Interactables)
    {
        if (!IsValid(Interactable))
            continue;

        const FVector CamToInteractable = (Interactable->GetComponentLocation() - CameraLocation);
        const float Alignment = FVector::DotProduct(CameraDirection.GetSafeNormal(), CamToInteractable.GetSafeNormal());   
        if(Alignment > BestAlignment){
            BestInteractable = Interactable;
            BestAlignment = Alignment;
        }

    }

    if(!BestInteractable)
        return;

#if MCO_DEBUG
    if(bDebug){
       DrawDebugLine(GetWorld(), CameraLocation, BestInteractable->GetComponentLocation(), FColor::Red,
                     false, 2.0f, 0U, 0.5f); 
        GEngine->AddOnScreenDebugMessage(191919, 2.f, FColor::Green, 
                                         FString::Printf(TEXT("Interactable Alignment: %.2f"), BestAlignment));
    }
#endif

    BestInteractable->InteractWith(Cast<APawn>(GetOwner()));
}

void UMCO_InteractionComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                               const FHitResult& SweepResult)
{
    if (UMCO_InteractableComponent* Interactable = OtherActor->GetComponentByClass<UMCO_InteractableComponent>())
    {
        Interactables.AddUnique(Interactable);

#if MCO_DEBUG
        if (bDebug){
            GEngine->AddOnScreenDebugMessage(696969, 5.f, FColor::Green,
                                             FString::Printf(TEXT("Interactables: %d"), Interactables.Num()));
        }
#endif
    }
}
void UMCO_InteractionComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (UMCO_InteractableComponent* Interactable = OtherActor->GetComponentByClass<UMCO_InteractableComponent>())
    {
        Interactables.Remove(Interactable);

#if MCO_DEBUG
        if (bDebug){
            GEngine->AddOnScreenDebugMessage(696969, 5.f, FColor::Red,
                                             FString::Printf(TEXT("Interactables: %d"), Interactables.Num()));
        }
#endif
    }
}
