#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MCO_FurnitureBase.generated.h"

class USoundAttenuation;
class USoundBase;

UCLASS()
class MOVINGCO_API AMCO_FurnitureBase : public AActor
{
	GENERATED_BODY()

public:
	AMCO_FurnitureBase();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Objective")
    bool bIsScorable = false;

    /** Server: track who is carrying us so bumps can be blamed on them */
    void AddHolder(APawn* Pawn);
    void RemoveHolder(APawn* Pawn);

    UFUNCTION(BlueprintPure, Category="_Furniture")
    bool IsHeld() const;

    /** Whoever is holding us, or dropped us a moment ago */
    APawn* GetResponsiblePawn() const;

protected:
	virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category="_Furniture")
    class UStaticMeshComponent* Mesh;

    /** Impact speed (cm/s change along the hit normal) needed for a hit to count as a bump */
    UPROPERTY(EditAnywhere, Category="_Furniture|Bump")
    float MinBumpSpeed = 120.f;

    /** Impact speed at which a bump is as loud as it gets */
    UPROPERTY(EditAnywhere, Category="_Furniture|Bump")
    float LoudBumpSpeed = 600.f;

    UPROPERTY(EditAnywhere, Category="_Furniture|Bump")
    float BumpCooldown = 0.35f;

    /** How long after letting go the last holder still gets blamed for bumps */
    UPROPERTY(EditAnywhere, Category="_Furniture|Bump")
    float BlameTime = 2.f;

    /** Leave empty to use a generated thud */
    UPROPERTY(EditAnywhere, Category="_Furniture|Bump")
    TObjectPtr<USoundBase> BumpSound;

    /** Leave empty to use a default falloff */
    UPROPERTY(EditAnywhere, Category="_Furniture|Bump")
    TObjectPtr<USoundAttenuation> BumpAttenuation;

public:
	virtual void Tick(float DeltaTime) override;

private:
    UFUNCTION()
    void OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                   FVector NormalImpulse, const FHitResult& Hit);

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayBump(FVector_NetQuantize Location, float Intensity);

    USoundAttenuation* GetBumpAttenuation();

    TArray<TWeakObjectPtr<APawn>> Holders;
    TWeakObjectPtr<APawn> LastHolder;
    float LastReleaseTime = -1000.f;
    float LastBumpTime = -1000.f;

    UPROPERTY(Transient)
    TObjectPtr<USoundAttenuation> DefaultBumpAttenuation;
};
