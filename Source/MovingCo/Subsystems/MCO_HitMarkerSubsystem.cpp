#include "MCO_HitMarkerSubsystem.h"

#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "NiagaraTextToolkitHelpers.h"

#include "Events/MCO_EventSubsystem.h"
#include "Settings/MCO_HitMarkerSettings.h"


void UMCO_HitMarkerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

    // Get the Niagara system to use
    HitMarkerSystem = GetDefault<UMCO_HitMarkerSettings>()->HitMarkerSystem.LoadSynchronous(); 

    // Bind to the OnValuableDamaged
    UMCO_EventSubsystem* EventSS = Collection.InitializeDependency<UMCO_EventSubsystem>();
    EventSS->GetAllEvents()->OnValuableDamaged.AddDynamic(this, &UMCO_HitMarkerSubsystem::HandleOnValuableDamaged);
}


bool UMCO_HitMarkerSubsystem::ShouldCreateSubsystem(UObject* Outer) const {
    const UWorld* World = Cast<UWorld>(Outer);
    return World and World->IsGameWorld() and !IsRunningDedicatedServer();
}


void UMCO_HitMarkerSubsystem::HandleOnValuableDamaged(float DamageValue, const FVector& WorldLocation){
    if(!HitMarkerSystem)
        return;

    UNiagaraComponent* Marker = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), HitMarkerSystem, WorldLocation, FRotator::ZeroRotator,
        FVector(1.f), /*bAutoDestroy*/ true, /*bAutoActivate*/ false, ENCPoolMethod::AutoRelease);

    if(!Marker) return;        

    UNiagaraTextToolkitHelpers::SetNiagaraNTTTextVariable(
        Marker, FString::FromInt(FMath::RoundToInt(DamageValue)));
    Marker->Activate(true);
}
