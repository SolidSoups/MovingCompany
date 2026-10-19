#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MCO_CityBuilding.generated.h"

/**
 * Procedurally assembles a multi story city building out of the modular city pieces.
 * Storefront ground floor, repeating upper floors, a stairwell in the back left corner, a flat roof with a parapet
 * and a sidewalk with a curb along the front.
 *
 * Wall runs are described by pattern strings, one character per 3m module, repeated to fill the side:
 *   S = solid wall, W = window (MCO_BP_Window), D = door (MCO_BP_Door),
 *   G = garage door (MCO_BP_GarageDoor, takes 2 modules), O = open (nothing).
 * A garage door that doesn't fit at the end of a run is replaced with solid wall.
 *
 * Local space: X runs along the street, the front facade sits at Y = 0 facing -Y, Z = 0 is the ground floor.
 */
UCLASS()
class MOVINGCO_API AMCO_CityBuilding : public AActor
{
    GENERATED_BODY()

public:
    AMCO_CityBuilding(const FObjectInitializer& ObjectInitializer);
    virtual void OnConstruction(const FTransform& Transform) override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
    TObjectPtr<class USceneComponent> SceneRootComp;

    // Instanced meshes, swap the mesh on these components to reskin the building.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building|Meshes")
    TObjectPtr<class UInstancedStaticMeshComponent> WallISM;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building|Meshes")
    TObjectPtr<class UInstancedStaticMeshComponent> FloorISM;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building|Meshes")
    TObjectPtr<class UInstancedStaticMeshComponent> CeilingISM;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building|Meshes")
    TObjectPtr<class UInstancedStaticMeshComponent> StairsISM;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building|Meshes")
    TObjectPtr<class UInstancedStaticMeshComponent> TrimISM;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building|Meshes")
    TObjectPtr<class UInstancedStaticMeshComponent> SidewalkISM;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building|Meshes")
    TObjectPtr<class UInstancedStaticMeshComponent> CurbISM;

    // Actor classes for the interactable wall modules.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Classes")
    TSubclassOf<AActor> DoorWallClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Classes")
    TSubclassOf<AActor> WindowWallClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Classes")
    TSubclassOf<AActor> GarageDoorWallClass;

    // Footprint in 3m modules and number of 4m stories.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Size", meta = (ClampMin = "1", ClampMax = "10"))
    int32 Width = 4;
    // At least two modules deep so the stairwell fits.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Size", meta = (ClampMin = "2", ClampMax = "10"))
    int32 Depth = 3;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Size", meta = (ClampMin = "1", ClampMax = "12"))
    int32 Stories = 4;

    // Ground floor patterns.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Layout")
    FString FrontGround = TEXT("WDG");
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Layout")
    FString BackGround = TEXT("SD");
    // The stairwell runs along the back two modules.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Layout")
    FString LeftGround = TEXT("WS");
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Layout")
    FString RightGround = TEXT("WS");

    // Upper floor patterns, used for every story above the ground floor.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Layout")
    FString FrontUpper = TEXT("W");
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Layout")
    FString BackUpper = TEXT("WS");
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Layout")
    FString LeftUpper = TEXT("WS");
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Layout")
    FString RightUpper = TEXT("W");

    // Rotates every wall 180 degrees, use if the exterior side of the walls ends up facing inwards.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Layout")
    bool bFlipWallFacing = false;

    // Rotates only the window modules 180 degrees, use if the windows face inwards while the walls are correct.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Layout")
    bool bFlipWindowFacing = true;

    // Yaw of the stair meshes, change in steps of 180 if the stairs climb the wrong way.
    // The flights always climb towards the back wall, this only turns the meshes.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Layout")
    float StairsYaw = 180.f;

    // Height of the low wall around the flat roof, measured from the top of the top story.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Roof", meta = (ClampMin = "0"))
    float ParapetHeight = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Street")
    bool bBuildSidewalk = true;
    // Distance from the front facade to the curb, the sidewalk fills the gap.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Street", meta = (ClampMin = "0", EditCondition = "bBuildSidewalk"))
    float CurbDistance = 300.f;

private:
    static constexpr float ModuleSize = 300.f;
    static constexpr float WallHeight = 400.f;

    void BuildWallRun(const FString& Pattern, const FVector& Start, const FVector& Direction, float Yaw, int32 MaxModules);
    void BuildCornerTrims();
    void BuildFloors();
    void BuildStairs();
    void BuildParapet();
    void BuildSidewalk();

    bool IsStairsCell(int32 X, int32 Y) const;

    // Adds an instance so that the mesh bounding box is centred on Center, regardless of the mesh pivot.
    void AddMeshInstance(class UInstancedStaticMeshComponent* ISM, const FVector& Center, const FQuat& Rotation, const FVector& Scale = FVector::OneVector);
    // Spawns a wall module actor so that its wall mesh sits bottom-centred on BottomCenter along the run.
    void AddWallActor(TSubclassOf<AActor> Class, const FVector& BottomCenter, float Yaw);

    static FVector CellCenter(int32 X, int32 Y, float Z);
};
