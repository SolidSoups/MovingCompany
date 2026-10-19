#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MCO_GriffinHouse.generated.h"

/**
 * Procedurally assembles the Griffin house (31 Spooner Street) out of the modular city pieces.
 * Two story house with a front porch, an attached single story garage on the right and a gable roof.
 *
 * Wall runs are described by pattern strings, one character per 3m module:
 *   S = solid wall, W = window (MCO_BP_Window), D = door (MCO_BP_Door),
 *   G = garage door (MCO_BP_GarageDoor, takes 2 modules), O = open (nothing).
 *
 * Local space: X runs along the street, the front facade sits at Y = 0 facing -Y, Z = 0 is the ground floor.
 */
UCLASS()
class MOVINGCO_API AMCO_GriffinHouse : public AActor
{
    GENERATED_BODY()

public:
    AMCO_GriffinHouse(const FObjectInitializer& ObjectInitializer);
    virtual void OnConstruction(const FTransform& Transform) override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "House")
    TObjectPtr<class USceneComponent> SceneRootComp;

    // Instanced meshes, swap the mesh on these components to reskin the house.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "House|Meshes")
    TObjectPtr<class UInstancedStaticMeshComponent> WallISM;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "House|Meshes")
    TObjectPtr<class UInstancedStaticMeshComponent> FloorISM;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "House|Meshes")
    TObjectPtr<class UInstancedStaticMeshComponent> CeilingISM;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "House|Meshes")
    TObjectPtr<class UInstancedStaticMeshComponent> RoofISM;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "House|Meshes")
    TObjectPtr<class UInstancedStaticMeshComponent> StairsISM;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "House|Meshes")
    TObjectPtr<class UInstancedStaticMeshComponent> TrimISM;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "House|Meshes")
    TObjectPtr<class UInstancedStaticMeshComponent> CurbISM;

    // Actor classes for the interactable wall modules.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Classes")
    TSubclassOf<AActor> DoorWallClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Classes")
    TSubclassOf<AActor> WindowWallClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Classes")
    TSubclassOf<AActor> GarageDoorWallClass;

    // Ground floor patterns.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Layout")
    FString FrontGround = TEXT("WWDWW");
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Layout")
    FString BackGround = TEXT("WSDSW");
    // The stairs run along the back two modules.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Layout")
    FString LeftGround = TEXT("WSS");
    // The first two modules border the garage.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Layout")
    FString RightGround = TEXT("DSW");

    // Upper floor patterns.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Layout")
    FString FrontUpper = TEXT("WWSWW");
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Layout")
    FString BackUpper = TEXT("WSWSW");
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Layout")
    FString LeftUpper = TEXT("WSW");
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Layout")
    FString RightUpper = TEXT("WSW");

    // Garage patterns, the garage is two modules wide and two modules deep.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Layout")
    FString GarageFront = TEXT("G");
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Layout")
    FString GarageRight = TEXT("SW");
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Layout")
    FString GarageBack = TEXT("SS");

    // Rotates every wall 180 degrees, use if the exterior side of the walls ends up facing inwards.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Layout")
    bool bFlipWallFacing = false;

    // Rotates only the window modules 180 degrees, use if the windows face inwards while the walls are correct.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Layout")
    bool bFlipWindowFacing = true;

    // Yaw of the stair meshes, change in steps of 180 if the stairs climb the wrong way.
    // The flights always climb towards the back wall, this only turns the meshes.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Layout")
    float StairsYaw = 180.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Layout")
    bool bBuildPorch = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Roof")
    bool bPitchedRoof = true;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Roof", meta = (ClampMin = "10", ClampMax = "60", EditCondition = "bPitchedRoof"))
    float RoofPitch = 35.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Roof", meta = (ClampMin = "0", EditCondition = "bPitchedRoof"))
    float RoofOverhang = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Street")
    bool bBuildCurb = true;
    // Distance from the front facade to the curb.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "House|Street", meta = (EditCondition = "bBuildCurb"))
    float CurbDistance = 900.f;

private:
    static constexpr float ModuleSize = 300.f;
    static constexpr float WallHeight = 400.f;
    static constexpr int32 HouseWidth = 5;
    static constexpr int32 HouseDepth = 3;
    static constexpr int32 GarageWidth = 2;
    static constexpr int32 GarageDepth = 2;

    void BuildWallRun(const FString& Pattern, const FVector& Start, const FVector& Direction, float Yaw, int32 MaxModules);
    void BuildCornerTrims();
    void BuildFloors();
    void BuildStairs();
    void BuildPorch();
    void BuildRoof();
    void BuildCurb();

    // Adds an instance so that the mesh bounding box is centred on Center, regardless of the mesh pivot.
    void AddMeshInstance(class UInstancedStaticMeshComponent* ISM, const FVector& Center, const FQuat& Rotation, const FVector& Scale = FVector::OneVector);
    // Spawns a wall module actor so that its wall mesh sits bottom-centred on BottomCenter along the run.
    void AddWallActor(TSubclassOf<AActor> Class, const FVector& BottomCenter, float Yaw);

    static FVector CellCenter(int32 X, int32 Y, float Z);
};
