#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MCO_ItemValueComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FMCO_OnValuableDepletedSignature);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MOVINGCO_API UMCO_ItemValueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMCO_ItemValueComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

    // Apply damage to the value of this item. Location is for cosmetics only.
    // [Server only]
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Valued Items")
    void DamageValuable(float Damage, const FVector& Location);

    // Called when the value has depleted past or equal to zero. [Server only]
    FMCO_OnValuableDepletedSignature OnValueDepleted;

    // The default value of the object
    UPROPERTY(EditAnywhere, Category="Item Value")
    float DefaultValue = 100.f;
protected:
	virtual void BeginPlay() override;

private:
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_OnValuableDamaged(float Damage, const FVector_NetQuantize10& Location);

    UPROPERTY(Replicated)
    float CurrentValue = 0.0f;
};
