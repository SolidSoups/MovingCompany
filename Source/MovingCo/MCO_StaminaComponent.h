#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MCO_StaminaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMCO_OnStaminaFullyDrained);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MOVINGCO_API UMCO_StaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMCO_StaminaComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void StartStaminaDrain(bool bLifting);
    void EndStaminaDrain();

    UPROPERTY(BlueprintAssignable, Category="MCO|Stamina")
    FMCO_OnStaminaFullyDrained OnStaminaFullyDrained;

protected:
	virtual void BeginPlay() override;
    virtual void EndPlay(EEndPlayReason::Type Reason) override;

    UPROPERTY(EditDefaultsOnly, Category="MCO|Stamina Widget")
    TSubclassOf<class UMCO_StaminaUserWidget> StaminaWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category="MCO|Stamina")
    float StartingStamina = 100.f;

    // Per-second
    UPROPERTY(EditDefaultsOnly, Category="MCO|Stamina")
    float DraggingStaminaDrain = 10.f;

    // Per-second
    UPROPERTY(EditDefaultsOnly, Category="MCO|Stamina")
    float LiftingStaminaDrain = 20.f;

    // Per-second
    UPROPERTY(EditDefaultsOnly, Category="MCO|Stamina")
    float StaminaRecoveryPerSecond = 30.f;


private:
    float CurrentStamina;

    bool bDraining = false;
    bool bLifting = false;

    UPROPERTY()
    TObjectPtr<class UMCO_StaminaUserWidget> StaminaWidget;

};
