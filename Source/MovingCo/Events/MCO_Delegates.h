#pragma once

#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "MCO_Delegates.generated.h"

class APawn;
class UMCO_InteractableComponent;

// All events
UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMCO_OnValuableDamagedSignature, float, ValueDamage, const FVector&, Location);

// Server events
UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMCO_OnInteractedSignature, APawn*, Instigator, UMCO_InteractableComponent*, Interactable);

// Local events
UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMCO_OnInteractionTargetChangedSignature, APawn*, Interactor, UMCO_InteractableComponent*, NewTarget);
