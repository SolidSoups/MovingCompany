#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MCO_StealthSubsystem.generated.h"

class AMCO_WatcherAIController;

/**
 *  Routes furniture bumps to everything that can see or hear them (customers, panicking cows).
 *  Server only.
 */
UCLASS()
class MOVINGCO_API UMCO_StealthSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
    void RegisterWatcher(AMCO_WatcherAIController* Watcher);
    void UnregisterWatcher(AMCO_WatcherAIController* Watcher);

    /** Something bumped into something. Loudness is 0-1, Offender is the pawn to blame (may be null) */
    void ReportBump(AActor* Source, const FVector& Location, float Loudness, APawn* Offender);

private:
    TArray<TWeakObjectPtr<AMCO_WatcherAIController>> Watchers;
};
