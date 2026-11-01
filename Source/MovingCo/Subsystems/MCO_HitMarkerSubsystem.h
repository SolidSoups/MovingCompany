#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MCO_HitMarkerSubsystem.generated.h"

UCLASS()
class MOVINGCO_API UMCO_HitMarkerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

private:
    UFUNCTION()
    void HandleOnValuableDamaged(float DamageValue, const FVector& WorldLocation);

    UPROPERTY()
    TObjectPtr<class UNiagaraSystem> HitMarkerSystem;
};
