#include "MCO_InteractableComponent.h"

#include "Components/SphereComponent.h"
#include "Events/MCO_EventSubsystem.h"
#include "MovingCo.h"
#include "Subsystems/MCO_InteractablesSubsystem.h"

UMCO_InteractableComponent::UMCO_InteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMCO_InteractableComponent::BeginPlay(){
    Super::BeginPlay();

    auto* InteractableSS = GetWorld()->GetSubsystem<UMCO_InteractablesSubsystem>();
    if(!InteractableSS){
        UE_LOG(LogMovingCo, Error, TEXT("Could not get interactable subsystem"));
        return;
    }
    else{
        InteractableSS->RegisterInteractable(this);
    }
}
void UMCO_InteractableComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
    auto* InteractableSS = GetWorld()->GetSubsystem<UMCO_InteractablesSubsystem>();
    if(!InteractableSS){
        UE_LOG(LogMovingCo, Error, TEXT("Could not get interactable subsystem"));
        return;
    }
    else{
        InteractableSS->UnregisterInteractable(this);
    }

    Super::EndPlay(EndPlayReason);
}

void UMCO_InteractableComponent::InteractWith(APawn* Instigator){
    OnInteractedWith.Broadcast(Instigator);

    // Server events are null on clients, so this only broadcasts on the server
    UMCO_EventSubsystem* EventSS = UMCO_EventSubsystem::Get(this);
    if(UMCO_ServerEvents* ServerEvents = EventSS ? EventSS->GetServerEvents() : nullptr){
        ServerEvents->OnInteracted.Broadcast(Instigator, this);
    }
}
