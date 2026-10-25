#pragma once

#include "CoreMinimal.h"
#include "MCO_Delegates.h"
#include "MCO_Events.generated.h"

// All-events for native c++.
USTRUCT()
struct FMCO_Native_EventsAllMachines {
    GENERATED_BODY() 

    // Called when a valuable (such as a furniture) loses monetary value, usually from damage.
    // Also provided is the location the damage stemmed from.
    FMCO_OnValuableDamagedNativeSignature OnValuableDamagedDelegate;
};

// All-events for blueprint.
// All-events are events that apply to all machines, like cosmetics.
// NOTE [Elias]: This needs to be a class so that blueprint passes by value instead of copying.
UCLASS()
class UMCO_Blueprint_EventsAllMachines : public UObject {
    GENERATED_BODY()

public:

    // Called when a valuable (such as a furniture) loses monetary value, usually from damage.
    // Also provided is the location the damage stemmed from.
    UPROPERTY(BlueprintAssignable) 
    FMCO_OnValuableDamagedDynamicSignature OnValuableDamagedDelegate;
};
