#include "MCO_InteractionComponent.h"

#include "Components/SphereComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/HitResult.h"
#include "MCO_InteractableComponent.h"
#include "MovingCo.h"

UMCO_InteractionComponent::UMCO_InteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
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
}


void UMCO_InteractionComponent::Server_DoInteraction_Implementation(){
    for(auto* Interactable : Interactables){
        if(IsValid(Interactable)){
            Interactable->InteractWith(GetOwner() ? Cast<APawn>(GetOwner()) : nullptr);
        }
    }
}


void UMCO_InteractionComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                               const FHitResult& SweepResult)
{
    if(UMCO_InteractableComponent* Interactable = OtherActor->GetComponentByClass<UMCO_InteractableComponent>()){
        Interactables.AddUnique(Interactable);
    }
}
void UMCO_InteractionComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if(UMCO_InteractableComponent* Interactable = OtherActor->GetComponentByClass<UMCO_InteractableComponent>()){
        Interactables.Remove(Interactable);
    }
}
