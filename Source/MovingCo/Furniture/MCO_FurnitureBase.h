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

protected:
	virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category="_Furniture")
    class UStaticMeshComponent* Mesh;


public:
	virtual void Tick(float DeltaTime) override;
};
