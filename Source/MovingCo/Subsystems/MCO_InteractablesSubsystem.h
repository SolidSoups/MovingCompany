#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MCO_InteractablesSubsystem.generated.h"

class UMCO_InteractableComponent;

UCLASS()
class MOVINGCO_API UMCO_InteractablesSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
    void RegisterInteractable(class UMCO_InteractableComponent* Component);
    void UnregisterInteractable(class UMCO_InteractableComponent* Component);

    UFUNCTION(BlueprintPure, Category="Interactables")
    const TArray<UMCO_InteractableComponent*>& GetInteractables() const { return Interactables; }

private:
    TArray<UMCO_InteractableComponent*> Interactables;
};
