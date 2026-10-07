#include "MCO_StaminaComponent.h"
#include "Blueprint/UserWidget.h"
#include "UI/MCO_StaminaUserWidget.h"

UMCO_StaminaComponent::UMCO_StaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMCO_StaminaComponent::BeginPlay() { 
    Super::BeginPlay(); 

    CurrentStamina = StartingStamina;

    // Spawn the widget
    if(!StaminaWidgetClass){
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
                                         TEXT("No stamina widget class"));
        return;
    }

    APlayerController* PC = nullptr;
    if(APawn* Pawn = Cast<APawn>(GetOwner())){
        PC = Pawn->GetController<APlayerController>();
    }
    else if(APlayerController* OwnerPC = Cast<APlayerController>(GetOwner()))
    { 
        PC = OwnerPC;
    }

    if(!PC or !PC->IsLocalPlayerController()){
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
                                         TEXT("No PC or PC isn't local player"));
        return;
    }

    StaminaWidget = CreateWidget<UMCO_StaminaUserWidget>(PC, StaminaWidgetClass);
    if(StaminaWidget){
        StaminaWidget->AddToViewport();
    }
    else{
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
                                         TEXT("Failed to create Stamina Widget"));
    }
}

void UMCO_StaminaComponent::EndPlay(EEndPlayReason::Type Reason) {
    if(StaminaWidget){
        StaminaWidget->RemoveFromParent();
        StaminaWidget = nullptr;
    }
}

void UMCO_StaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if(bDraining){
        const float PreviousStamina = CurrentStamina;

        float DrainPerSecond = bLifting 
            ? LiftingStaminaDrain 
            : DraggingStaminaDrain;
        CurrentStamina = FMath::Clamp(
            CurrentStamina - DrainPerSecond * DeltaTime,
            0.f, StartingStamina
        );

        if(PreviousStamina > 0.f and CurrentStamina <= 0.f)
            OnStaminaFullyDrained.Broadcast();
    }
    else{
        CurrentStamina = FMath::Clamp(
            CurrentStamina + StaminaRecoveryPerSecond * DeltaTime,
            0.f, StartingStamina
        );
    }

    if(StaminaWidget)
        StaminaWidget->SetStaminaFill(CurrentStamina / StartingStamina);
}


void UMCO_StaminaComponent::EndStaminaDrain() {
    bDraining = false;
    bLifting = false;
}
void UMCO_StaminaComponent::StartStaminaDrain(bool bInLifting) {
    bDraining = true;
    bLifting = bInLifting;
}
