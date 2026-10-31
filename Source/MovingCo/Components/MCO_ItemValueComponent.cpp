#include "MCO_ItemValueComponent.h"
#include "Events/MCO_EventSubsystem.h"
#include "Net/UnrealNetwork.h"

UMCO_ItemValueComponent::UMCO_ItemValueComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UMCO_ItemValueComponent::BeginPlay()
{
	Super::BeginPlay();

    if(GetOwner()->HasAuthority()){
        CurrentValue = DefaultValue;
    }
}

void UMCO_ItemValueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UMCO_ItemValueComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UMCO_ItemValueComponent, CurrentValue);
}

void UMCO_ItemValueComponent::DamageValuable(float Damage, const FVector &Location){
    if(!GetOwner()->HasAuthority())
        return;

    Multicast_OnValuableDamaged(Damage, Location);

    CurrentValue -= Damage;
    if (CurrentValue <= 0.0f){
        OnValueDepleted.Broadcast();
    }
}

void UMCO_ItemValueComponent::Multicast_OnValuableDamaged_Implementation(float Damage, const FVector_NetQuantize10 &Location){
    if(UMCO_EventSubsystem* EventSS = UMCO_EventSubsystem::Get(this)){
        EventSS->GetAllEvents()->OnValuableDamaged.Broadcast(Damage, Location);
    }
}

