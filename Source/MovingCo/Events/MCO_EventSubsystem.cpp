#include "MCO_EventSubsystem.h"

void UMCO_EventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // The blueprint events classes have to be created during runtime
    // as they are UObjects
    if(!IsValid(DynamicEvents_AllMachines)) 
        DynamicEvents_AllMachines = NewObject<UMCO_Blueprint_EventsAllMachines>();
    
    // Bind all cpp native events to the blueprint equivelents
    SetupBlueprintAllEvents();
    SetupBlueprintLocalEvents();
    if(GetWorld()->GetNetMode() != NM_Client){
        // These events are only needed on the server
        SetupBlueprintServerEvents();
    }
}


void UMCO_EventSubsystem::SetupBlueprintAllEvents(){
    // Forward broadcasts from the native events to the dynamic events

    NativeEvents_AllMachines.OnValuableDamagedDelegate.AddLambda([this](float ValueDamage, const FVector& Location){ DynamicEvents_AllMachines->OnValuableDamagedDelegate.Broadcast(ValueDamage, Location); });
}
void UMCO_EventSubsystem::SetupBlueprintLocalEvents(){
    // Forward broadcasts from the native events to the dynamic events
    
}
void UMCO_EventSubsystem::SetupBlueprintServerEvents(){
    // Forward broadcasts from the native events to the dynamic events
    
}
