#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MCO_CustomerCharacter.generated.h"

class UMCO_VisionConeComponent;

/**
 *  The customer whose house is being moved into. Walks its patrol route, comes to look when it hears
 *  furniture bumping into things, and stresses out any mover it catches doing it.
 *  Driven by AMCO_WatcherAIController, so the level needs a nav mesh.
 */
UCLASS()
class MOVINGCO_API AMCO_CustomerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMCO_CustomerCharacter();

    int32 GetNumPatrolPoints() const { return PatrolPoints.Num(); }

    /** World location of a patrol point, as it was placed relative to where the customer started */
    FVector GetPatrolPoint(int32 Index) const;

    UMCO_VisionConeComponent* GetVisionCone() const { return VisionCone; }

protected:
    /** Visited in order, then loops. Relative to the customer's start, drag the widgets around in the level */
    UPROPERTY(EditInstanceOnly, Category="MCO|Patrol", meta=(MakeEditWidget))
    TArray<FVector> PatrolPoints;

private:
    UPROPERTY(VisibleAnywhere, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UMCO_VisionConeComponent> VisionCone;

    // The actor moves while patrolling, so the route is pinned to the start transform
    mutable TOptional<FTransform> PatrolOrigin;
};
