#include "MCO_InteractableComponent.h"

#include "Components/SphereComponent.h"

UMCO_InteractableComponent::UMCO_InteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UMCO_InteractableComponent::BeginPlay(){
    Super::BeginPlay();

    if(GetOwner()->HasAuthority()){
        SphereComp = NewObject<USphereComponent>(GetOwner(), TEXT("Interaction Zone")); 
        SphereComp->InitSphereRadius(300.f);
        // TODO: Create custom collision profile for only interactables
        SphereComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
        SphereComp->SetupAttachment(this);
        SphereComp->RegisterComponent();
    }
}

void UMCO_InteractableComponent::InteractWith(APawn* Instigator){
    OnInteractedWith.Broadcast(Instigator);
}
