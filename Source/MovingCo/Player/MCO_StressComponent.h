#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MCO_StressComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMCO_OnPanicChanged, bool, bPanicking);

/**
 *  The cow's health. Fills up when the customer catches you bumping their stuff or stares you down.
 *  When it's full the cow panics until it has calmed all the way back down.
 *  Server authoritative, replicated to everyone.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MOVINGCO_API UMCO_StressComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMCO_StressComponent();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Ignored while already panicking */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="MCO|Stress")
    void AddStress(float Amount);

    UFUNCTION(BlueprintPure, Category="MCO|Stress")
    float GetStressPercent() const { return MaxStress > 0.f ? Stress / MaxStress : 0.f; }

    UFUNCTION(BlueprintPure, Category="MCO|Stress")
    bool IsPanicking() const { return bPanicking; }

    /** Fires on server and clients */
    UPROPERTY(BlueprintAssignable, Category="MCO|Stress")
    FMCO_OnPanicChanged OnPanicChanged;

protected:
    virtual void EndPlay(EEndPlayReason::Type Reason) override;

    UPROPERTY(EditDefaultsOnly, Category="MCO|Stress Widget")
    TSubclassOf<class UMCO_StressUserWidget> StressWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category="MCO|Stress", meta=(ClampMin="1"))
    float MaxStress = 100.f;

    /** Seconds without new stress before the cow starts calming down */
    UPROPERTY(EditDefaultsOnly, Category="MCO|Stress")
    float CalmDelay = 3.f;

    UPROPERTY(EditDefaultsOnly, Category="MCO|Stress")
    float CalmPerSecond = 5.f;

    /** How fast stress drains while panicking. Panic ends once it's back at zero */
    UPROPERTY(EditDefaultsOnly, Category="MCO|Stress")
    float PanicCalmPerSecond = 7.f;

private:
    void SetPanicking(bool bInPanicking);
    void UpdateWidget();

    UFUNCTION()
    void OnRep_Panicking();

    UPROPERTY(Replicated)
    float Stress = 0.f;

    UPROPERTY(ReplicatedUsing=OnRep_Panicking)
    bool bPanicking = false;

    float TimeSinceStressed = 0.f;

    UPROPERTY()
    TObjectPtr<class UMCO_StressUserWidget> StressWidget;
};
