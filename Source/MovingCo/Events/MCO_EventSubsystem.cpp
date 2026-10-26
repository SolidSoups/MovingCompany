#include "MCO_EventSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/World.h"

UMCO_EventSubsystem* UMCO_EventSubsystem::Get(const UObject* WorldContextObject)
{
    const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    return World ? World->GetSubsystem<UMCO_EventSubsystem>() : nullptr;
}

void UMCO_EventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // The event classes have to be created during runtime as they are UObjects
    AllEvents = NewObject<UMCO_AllEvents>(this);
    ServerEvents = NewObject<UMCO_ServerEvents>(this);
    LocalEvents = NewObject<UMCO_LocalEvents>(this);
}

UMCO_ServerEvents* UMCO_EventSubsystem::GetServerEvents() const
{
    // The net mode is checked on access, as it is not reliable yet during Initialize
    const UWorld* World = GetWorld();
    if(!World || World->GetNetMode() == NM_Client)
        return nullptr;

    return ServerEvents;
}

bool UMCO_EventSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
    return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}
