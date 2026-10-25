#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MCO_FurnitureBase.generated.h"

UCLASS()
class MOVINGCO_API AMCO_FurnitureBase : public AActor
{
	GENERATED_BODY()

public:
	AMCO_FurnitureBase();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Objective")
    bool bIsScorable = false;
protected:
	virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category="Furniture")
    class UStaticMeshComponent* Mesh;

    UPROPERTY(VisibleAnywhere, Category="Furniture")
    TObjectPtr<class UMCO_ItemValueComponent> ItemValueComponent;

    // The impact speed of a very small bump
    UPROPERTY(EditDefaultsOnly, Category="Furniture|Impact")
    float MinImpactSpeed = 100.f;

    // The impact speed of a very large bump
    UPROPERTY(EditDefaultsOnly, Category="Furniture|Impact")
    float MaxImpactSpeed = 600.f;

    // The delay between impacts
    UPROPERTY(EditDefaultsOnly, Category="Furniture|Impact")
    double ImpactDelay = 0.35f;

    // Broadcast when the blueprint needs arrange FX for an impact
    UFUNCTION(BlueprintImplementableEvent, Category="Furniture|Impact")
    void PlayImpactFX(const FVector& Location, const FVector& NormalImpulse, float IntensityNorm);

private:
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayImpactFX(FVector_NetQuantize10 Location,
                                FVector_NetQuantize10 NormalImpulse,
                                float IntensityNorm);

    // Detects hits on mesh for damage + fx playback
    UFUNCTION()
    void OnMeshHit(
        UPrimitiveComponent* HitComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        FVector NormalImpulse,
        const FHitResult& Hit );

    double LastImpactTime = 0.0f;
};
