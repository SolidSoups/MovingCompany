#include "MCO_InteractablesSubsystem.h"
#include "Components/MCO_InteractableComponent.h"

void UMCO_InteractablesSubsystem::RegisterInteractable(UMCO_InteractableComponent* Component)
{
    if(IsValid(Component)){
        Interactables.AddUnique(Component);
    }
}
void UMCO_InteractablesSubsystem::UnregisterInteractable(UMCO_InteractableComponent* Component)
{
    if(IsValid(Component)){
        Interactables.Remove(Component);
    }
}
