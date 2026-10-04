#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MCO_DebrisActor.generated.h"

class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;

UCLASS()
class MOVINGCO_API AMCO_DebrisActor : public AActor
{
	GENERATED_BODY()

public:
	AMCO_DebrisActor();

    // Copies the look of the piece it replaces and starts simulating
    void Init(UStaticMesh* InMesh, UMaterialInterface* Material);

    // Pushes teh debris at a world location
    void Launch(const FVector& Impulse, const FVector& Location);

private:
    UPROPERTY(VisibleAnywhere, Category="Components", meta=(AllowPrivateAccess="true"))
    UStaticMeshComponent* Mesh;
};
