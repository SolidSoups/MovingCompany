#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MCO_PropBase.generated.h"

UCLASS()
class MOVINGCO_API AMCO_PropBase : public AActor
{
	GENERATED_BODY()

public:
	AMCO_PropBase();
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void TornOff() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Objective")
    bool bIsScorable = false;

    FVector GetLastVelocity() const { return LastVelocity; }
protected:
	virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, Category="Prop")
    class UStaticMeshComponent* Mesh;

    UPROPERTY(VisibleAnywhere, Category="Prop")
    TObjectPtr<class UMCO_ItemValueComponent> ItemValueComponent;

    // The impact speed of a very small bump
    UPROPERTY(EditDefaultsOnly, Category="Prop|Impact")
    float MinImpactSpeed = 100.f;

    // The impact speed of a very large bump
    UPROPERTY(EditDefaultsOnly, Category="Prop|Impact")
    float MaxImpactSpeed = 600.f;

    // The delay between impacts
    UPROPERTY(EditDefaultsOnly, Category="Prop|Impact")
    double ImpactDelay = 0.35f;

    // Called when the blueprint needs arrange FX for an impact
    UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category="Prop|Impact")
    void PlayImpactFX(const FVector& Location, const FVector& NormalImpulse, float IntensityNorm);

    // Called when you need to make FX for a destroyed prop
    UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category="Prop|Impact")
    void PlayDestroyFX();

private:
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayImpactFX(FVector_NetQuantize10 Location,
                                FVector_NetQuantize10 NormalImpulse,
                                float IntensityNorm);

    UFUNCTION()
    void HandleOnValueDepleted();

    // Detects hits on mesh for damage + fx playback
    UFUNCTION()
    void OnMeshHit(
        UPrimitiveComponent* HitComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        FVector NormalImpulse,
        const FHitResult& Hit );

    // Track the last time this prop was impacted,
    // to avoid LOTS of small impacts. 
    double LastImpactTime = 0.0f;

    // This flag is set on the server when the item value has depleted past zero
    UPROPERTY(ReplicatedUsing=OnRep_bIsDepleted)
    bool bIsDepleted = false;

    // The on rep function handles the cosmetic side of the value depletion.
    // It is faster than TornOff
    UFUNCTION()
    void OnRep_bIsDepleted();

    // The last known velocity of this object, before any forces where applied.
    // Useful for physics calculations in Mesh hit callbacks.
    FVector LastVelocity;
};
