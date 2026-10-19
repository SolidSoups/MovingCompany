#include "MCO_StealthSubsystem.h"
#include "MCO_WatcherAIController.h"

void UMCO_StealthSubsystem::RegisterWatcher(AMCO_WatcherAIController* Watcher)
{
    Watchers.AddUnique(Watcher);
}

void UMCO_StealthSubsystem::UnregisterWatcher(AMCO_WatcherAIController* Watcher)
{
    Watchers.Remove(Watcher);
}

void UMCO_StealthSubsystem::ReportBump(AActor* Source, const FVector& Location, float Loudness, APawn* Offender)
{
    Watchers.RemoveAll([](const TWeakObjectPtr<AMCO_WatcherAIController>& Watcher) { return !Watcher.IsValid(); });

    // Copy, a watcher reacting can end up (un)registering others
    const TArray<TWeakObjectPtr<AMCO_WatcherAIController>> Current = Watchers;
    for (const TWeakObjectPtr<AMCO_WatcherAIController>& Watcher : Current)
    {
        if (Watcher.IsValid())
            Watcher->HandleBump(Source, Location, Loudness, Offender);
    }
}
