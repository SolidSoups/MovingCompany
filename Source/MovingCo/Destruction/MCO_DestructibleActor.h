#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MCO_DestructibleActor.generated.h"

class UInstancedStaticMeshComponent;
class AMCO_DebrisActor;

struct FDestructiblePiece {
    FTransform Transform;
    TArray<int32> Neighbors;
    bool bIsAnchored = false;
    bool bIsDetached = false;
    float Damage = 0.f;
};

UCLASS()
class MOVINGCO_API AMCO_DestructibleActor : public AActor
{
	GENERATED_BODY()

public:
	AMCO_DestructibleActor();
    
    UFUNCTION(BlueprintCallable, Category="Destruction")
    void ApplyImpact(int32 PieceIndex, FVector Location, FVector Impulse);
    void ApplyRadialImpact(FVector Location, float Radius, FVector PushDirection, float ImpulseStrength);

    virtual void Reset() override;

protected:
    virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Category="Destruction", meta=(ClampMin="1"))
    int32 Rows = 10;

    UPROPERTY(EditAnywhere, Category="Destruction", meta=(ClampMin="1"))
    int32 Columns = 8;

    UPROPERTY(EditAnywhere, Category="Destruction")
    FVector BrickSize{20.f, 10.f, 6.f};

    UPROPERTY(EditAnywhere, Category="Destruction")
    float BrickGap = 0.5f;

    UPROPERTY(EditAnywhere, Category="Destruction", meta=(ClampMin="0"))
    float NeighborTolerance = 1.0f;

    UPROPERTY(EditAnywhere, Category="Destruction", meta=(ClampMin="0"))
    float DetachThreshold = 1000.f;

    TArray<FDestructiblePiece> Pieces;
    TArray<TWeakObjectPtr<AMCO_DebrisActor>> SpawnedDebris;

private:
    void BuildPieces();
    AMCO_DebrisActor* DetachPiece(int32 PieceIndex);
    void DetachUnsupportedPieces();

    UPROPERTY(VisibleAnywhere, Category="Components", meta=(AllowPrivateAccess="true"))
    UInstancedStaticMeshComponent* Bricks;
};
