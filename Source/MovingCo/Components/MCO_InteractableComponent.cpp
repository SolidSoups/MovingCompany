#include "MCO_InteractableComponent.h"

#include "Components/SphereComponent.h"

UMCO_InteractableComponent::UMCO_InteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

    SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("Interaction Overlap Zone"));
    SphereComp->SetupAttachment(this);
    SphereComp->InitSphereRadius(100.f);
    // TODO: Create custom collision profile for only interactables
    SphereComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void UMCO_InteractableComponent::InteractWith(APawn* Instigator){
    OnInteractedWith.Broadcast(Instigator);
}
