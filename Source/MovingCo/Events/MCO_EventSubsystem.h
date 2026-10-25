#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MCO_Events.h"
#include "MCO_EventSubsystem.generated.h"

// Manages events local to this machine.
// Please make sure only to bind to the dynamic events on the blueprint side.
UCLASS()
class MOVINGCO_API UMCO_EventSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UPROPERTY()
    FMCO_Native_EventsAllMachines NativeEvents_AllMachines;
    UPROPERTY(BlueprintReadWrite, Category="Events")
    TObjectPtr<UMCO_Blueprint_EventsAllMachines> DynamicEvents_AllMachines;

private:
    void SetupBlueprintAllEvents();
    void SetupBlueprintLocalEvents();
    void SetupBlueprintServerEvents();
};
