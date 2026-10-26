#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MCO_Delegates.h"
#include "MCO_Events.generated.h"

// NOTE [Elias]: These need to be classes so that blueprint passes by reference instead of copying.

// NetMulti-events called by both host and clients
UCLASS(BlueprintType)
class MOVINGCO_API UMCO_AllEvents : public UObject {
    GENERATED_BODY()

public:
    // Called when a valuable (such as a furniture) loses monetary value, usually from damage.
    // Also provided is the location the damage stemmed from.
    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Events")
    FMCO_OnValuableDamagedSignature OnValuableDamaged;
};

// Server-events only exist on the server.
UCLASS(BlueprintType)
class MOVINGCO_API UMCO_ServerEvents : public UObject {
    GENERATED_BODY()

public:
    // Called when a pawn has interacted with an interactable.
    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Events")
    FMCO_OnInteractedSignature OnInteracted;
};

// Local-events.
UCLASS(BlueprintType)
class MOVINGCO_API UMCO_LocalEvents : public UObject {
    GENERATED_BODY()

public:
    // Called when a locally controlled pawn starts or stops looking at an interactable.
    // NewTarget is null when the pawn no longer has a target.
    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Events")
    FMCO_OnInteractionTargetChangedSignature OnInteractionTargetChanged;
};
