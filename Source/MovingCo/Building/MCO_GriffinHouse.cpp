#include "Building/MCO_GriffinHouse.h"
#include "Components/ChildActorComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

namespace MCO_GriffinHouse
{
    // The stairs run through these ground floor cells, the upper floor leaves them open.
    constexpr int32 StairsCellX = 0;
    constexpr int32 StairsCellsY[] = { 1, 2 };
    constexpr int32 GableSteps = 6;

    FVector GetMeshSize(const UInstancedStaticMeshComponent* ISM, const FVector& Fallback)
    {
        const UStaticMesh* Mesh = ISM ? ISM->GetStaticMesh() : nullptr;
        return Mesh ? Mesh->GetBoundingBox().GetSize() : Fallback;
    }
}

AMCO_GriffinHouse::AMCO_GriffinHouse(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = false;
    // Replicated so clients don't spawn local copies of the (replicated) door and window child actors.
    bReplicates = true;
    SetReplicatingMovement(false);

    SceneRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
    SceneRootComp->SetMobility(EComponentMobility::Static);
    SetRootComponent(SceneRootComp);

    auto CreateISM = [this](const FName Name, const TCHAR* MeshPath)
    {
        UInstancedStaticMeshComponent* ISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(Name);
        ISM->SetupAttachment(SceneRootComp);
        ISM->SetMobility(EComponentMobility::Static);
        ISM->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

        ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(MeshPath);
        if (Mesh.Succeeded())
            ISM->SetStaticMesh(Mesh.Object);
        return ISM;
    };

    WallISM = CreateISM(TEXT("Walls"), TEXT("/Game/Art/Modular_Pieces/City/SM_City_Wall_Straight_3x4_A.SM_City_Wall_Straight_3x4_A"));
    FloorISM = CreateISM(TEXT("Floors"), TEXT("/Game/Art/Modular_Pieces/City/SM_City_Floor_3x3.SM_City_Floor_3x3"));
    CeilingISM = CreateISM(TEXT("Ceilings"), TEXT("/Game/Art/Modular_Pieces/City/SM_City_Ceiling_3x4_A.SM_City_Ceiling_3x4_A"));
    RoofISM = CreateISM(TEXT("Roof"), TEXT("/Game/Art/Modular_Pieces/City/SM_City_Ceiling_3x4_A.SM_City_Ceiling_3x4_A"));
    StairsISM = CreateISM(TEXT("Stairs"), TEXT("/Game/Art/Modular_Pieces/City/SM_City_Stairs_A.SM_City_Stairs_A"));
    TrimISM = CreateISM(TEXT("Trims"), TEXT("/Game/Art/Modular_Pieces/City/SM_Wall_Trim_Vertical_A.SM_Wall_Trim_Vertical_A"));
    CurbISM = CreateISM(TEXT("Curbs"), TEXT("/Game/Art/Modular_Pieces/City/SM_City_Curb_3m_A.SM_City_Curb_3m_A"));

    static ConstructorHelpers::FClassFinder<AActor> DoorClassFinder(TEXT("/Game/Blueprints/Actors/Door/MCO_BP_Door"));
    static ConstructorHelpers::FClassFinder<AActor> WindowClassFinder(TEXT("/Game/Blueprints/Actors/Door/MCO_BP_Window"));
    static ConstructorHelpers::FClassFinder<AActor> GarageDoorClassFinder(TEXT("/Game/Blueprints/Actors/Door/MCO_BP_GarageDoor"));
    DoorWallClass = DoorClassFinder.Class;
    WindowWallClass = WindowClassFinder.Class;
    GarageDoorWallClass = GarageDoorClassFinder.Class;
}

void AMCO_GriffinHouse::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    for (UInstancedStaticMeshComponent* ISM : { WallISM, FloorISM, CeilingISM, RoofISM, StairsISM, TrimISM, CurbISM })
        ISM->ClearInstances();

    const float Flip = bFlipWallFacing ? 180.f : 0.f;
    const float Width = HouseWidth * ModuleSize;
    const float Depth = HouseDepth * ModuleSize;

    // House, wall yaw is chosen so the exterior faces outwards (-Y at yaw 0).
    for (int32 Story = 0; Story < 2; ++Story)
    {
        const float Z = Story * WallHeight;
        BuildWallRun(Story == 0 ? FrontGround : FrontUpper, FVector(0.f, 0.f, Z), FVector::XAxisVector, Flip, HouseWidth);
        BuildWallRun(Story == 0 ? BackGround : BackUpper, FVector(0.f, Depth, Z), FVector::XAxisVector, 180.f + Flip, HouseWidth);
        BuildWallRun(Story == 0 ? LeftGround : LeftUpper, FVector(0.f, 0.f, Z), FVector::YAxisVector, -90.f + Flip, HouseDepth);
        BuildWallRun(Story == 0 ? RightGround : RightUpper, FVector(Width, 0.f, Z), FVector::YAxisVector, 90.f + Flip, HouseDepth);
    }

    // Garage, attached to the right side of the house. Its left side is the house wall.
    BuildWallRun(GarageFront, FVector(Width, 0.f, 0.f), FVector::XAxisVector, Flip, GarageWidth);
    BuildWallRun(GarageRight, FVector(Width + GarageWidth * ModuleSize, 0.f, 0.f), FVector::YAxisVector, 90.f + Flip, GarageDepth);
    BuildWallRun(GarageBack, FVector(Width, GarageDepth * ModuleSize, 0.f), FVector::XAxisVector, 180.f + Flip, GarageWidth);

    BuildCornerTrims();
    BuildFloors();
    BuildStairs();

    if (bBuildPorch)
        BuildPorch();
    if (bPitchedRoof)
        BuildRoof();
    if (bBuildCurb)
        BuildCurb();
}

void AMCO_GriffinHouse::BuildWallRun(const FString& Pattern, const FVector& Start, const FVector& Direction, float Yaw, int32 MaxModules)
{
    const FQuat Rotation(FRotator(0.f, Yaw, 0.f));

    int32 Slot = 0;
    for (const TCHAR RawChar : Pattern)
    {
        const TCHAR Char = FChar::ToUpper(RawChar);
        const int32 Modules = Char == TEXT('G') ? 2 : 1;
        if (Slot + Modules > MaxModules)
            break;

        const FVector BottomCenter = Start + Direction * (ModuleSize * (Slot + Modules * 0.5f));
        switch (Char)
        {
        case TEXT('S'):
            AddMeshInstance(WallISM, BottomCenter + FVector(0.f, 0.f, WallHeight * 0.5f), Rotation);
            break;
        case TEXT('W'):
            AddWallActor(WindowWallClass, BottomCenter, Yaw + (bFlipWindowFacing ? 180.f : 0.f));
            break;
        case TEXT('D'):
            AddWallActor(DoorWallClass, BottomCenter, Yaw);
            break;
        case TEXT('G'):
            AddWallActor(GarageDoorWallClass, BottomCenter, Yaw);
            break;
        default:
            break;
        }
        Slot += Modules;
    }
}

void AMCO_GriffinHouse::BuildCornerTrims()
{
    const float Width = HouseWidth * ModuleSize;
    const float Depth = HouseDepth * ModuleSize;
    const float GarageRightX = Width + GarageWidth * ModuleSize;
    const float HalfHeight = WallHeight * 0.5f;

    for (int32 Story = 0; Story < 2; ++Story)
    {
        const float Z = Story * WallHeight + HalfHeight;
        for (const FVector2D& Corner : { FVector2D(0.f, 0.f), FVector2D(Width, 0.f), FVector2D(0.f, Depth), FVector2D(Width, Depth) })
            AddMeshInstance(TrimISM, FVector(Corner, Z), FQuat::Identity);
    }

    AddMeshInstance(TrimISM, FVector(GarageRightX, 0.f, HalfHeight), FQuat::Identity);
    AddMeshInstance(TrimISM, FVector(GarageRightX, GarageDepth * ModuleSize, HalfHeight), FQuat::Identity);
}

void AMCO_GriffinHouse::BuildFloors()
{
    const float FloorThickness = MCO_GriffinHouse::GetMeshSize(FloorISM, FVector(300.f, 300.f, 30.f)).Z;
    const float CeilingThickness = MCO_GriffinHouse::GetMeshSize(CeilingISM, FVector(300.f, 300.f, 50.f)).Z;

    auto IsStairsCell = [](int32 X, int32 Y)
    {
        for (const int32 StairsY : MCO_GriffinHouse::StairsCellsY)
            if (X == MCO_GriffinHouse::StairsCellX && Y == StairsY)
                return true;
        return false;
    };

    for (int32 X = 0; X < HouseWidth; ++X)
    {
        for (int32 Y = 0; Y < HouseDepth; ++Y)
        {
            // Ground floor top at Z = 0, upper floor slab sits on the ground floor walls.
            AddMeshInstance(FloorISM, CellCenter(X, Y, -FloorThickness * 0.5f), FQuat::Identity);
            if (!IsStairsCell(X, Y))
                AddMeshInstance(FloorISM, CellCenter(X, Y, WallHeight + FloorThickness * 0.5f), FQuat::Identity);

            AddMeshInstance(CeilingISM, CellCenter(X, Y, WallHeight * 2.f + CeilingThickness * 0.5f), FQuat::Identity);
        }
    }

    for (int32 X = HouseWidth; X < HouseWidth + GarageWidth; ++X)
    {
        for (int32 Y = 0; Y < GarageDepth; ++Y)
        {
            AddMeshInstance(FloorISM, CellCenter(X, Y, -FloorThickness * 0.5f), FQuat::Identity);
            AddMeshInstance(CeilingISM, CellCenter(X, Y, WallHeight + CeilingThickness * 0.5f), FQuat::Identity);
        }
    }
}

void AMCO_GriffinHouse::BuildStairs()
{
    if (!StairsISM->GetStaticMesh())
        return;

    const FVector StairsSize = StairsISM->GetStaticMesh()->GetBoundingBox().GetSize();
    const float WallHalfThickness = MCO_GriffinHouse::GetMeshSize(WallISM, FVector(300.f, 50.f, 400.f)).Y * 0.5f;
    const FQuat Rotation(FRotator(0.f, StairsYaw, 0.f));
    const FVector RunDirection = FVector::YAxisVector;

    // Two flights back to back against the left wall, ending at the back wall.
    const FVector RunMiddle(
        MCO_GriffinHouse::StairsCellX * ModuleSize + WallHalfThickness + StairsSize.X * 0.5f,
        HouseDepth * ModuleSize - WallHalfThickness - StairsSize.Y,
        0.f);

    for (int32 Flight = 0; Flight < 2; ++Flight)
    {
        FVector Center = RunMiddle + RunDirection * StairsSize.Y * (Flight - 0.5f);
        Center.Z = StairsSize.Z * (Flight + 0.5f);
        AddMeshInstance(StairsISM, Center, Rotation);
    }
}

void AMCO_GriffinHouse::BuildPorch()
{
    int32 DoorIndex = INDEX_NONE;
    if (!FrontGround.FindChar(TEXT('D'), DoorIndex) && !FrontGround.FindChar(TEXT('d'), DoorIndex))
        return;

    const float FloorThickness = MCO_GriffinHouse::GetMeshSize(FloorISM, FVector(300.f, 300.f, 30.f)).Z;
    const float CeilingThickness = MCO_GriffinHouse::GetMeshSize(CeilingISM, FVector(300.f, 300.f, 50.f)).Z;
    const float TrimHalfSize = MCO_GriffinHouse::GetMeshSize(TrimISM, FVector(75.f, 75.f, 400.f)).X * 0.5f;

    AddMeshInstance(FloorISM, CellCenter(DoorIndex, -1, -FloorThickness * 0.5f), FQuat::Identity);
    AddMeshInstance(CeilingISM, CellCenter(DoorIndex, -1, WallHeight + CeilingThickness * 0.5f), FQuat::Identity);

    // Columns holding up the porch roof.
    const float ColumnY = -ModuleSize + TrimHalfSize;
    AddMeshInstance(TrimISM, FVector(DoorIndex * ModuleSize + TrimHalfSize, ColumnY, WallHeight * 0.5f), FQuat::Identity);
    AddMeshInstance(TrimISM, FVector((DoorIndex + 1) * ModuleSize - TrimHalfSize, ColumnY, WallHeight * 0.5f), FQuat::Identity);
}

void AMCO_GriffinHouse::BuildRoof()
{
    const FVector RoofMeshSize = MCO_GriffinHouse::GetMeshSize(RoofISM, FVector(300.f, 300.f, 50.f));
    const FVector WallMeshSize = MCO_GriffinHouse::GetMeshSize(WallISM, FVector(300.f, 50.f, 400.f));

    const float Width = HouseWidth * ModuleSize;
    const float Depth = HouseDepth * ModuleSize;
    const float HalfDepth = Depth * 0.5f;
    const float RoofBase = WallHeight * 2.f;
    const float Pitch = FMath::DegreesToRadians(RoofPitch);
    const float SlopeLength = (HalfDepth + RoofOverhang) / FMath::Cos(Pitch);
    const FVector Scale((Width + RoofOverhang * 2.f) / RoofMeshSize.X, SlopeLength / RoofMeshSize.Y, 1.f);

    // Each slope rises from the eave towards the ridge, which runs parallel to the street.
    // Slab centre is offset along its normal so the underside lies on the slope line.
    const float SlopeCenterY = (HalfDepth - RoofOverhang) * 0.5f;
    const float SlopeCenterZ = RoofBase + SlopeCenterY * FMath::Tan(Pitch);
    const float HalfThickness = RoofMeshSize.Z * 0.5f;

    const FQuat FrontRotation(FVector::XAxisVector, Pitch);
    const FVector FrontCenter(Width * 0.5f, SlopeCenterY, SlopeCenterZ);
    AddMeshInstance(RoofISM, FrontCenter + FrontRotation.GetUpVector() * HalfThickness, FrontRotation, Scale);

    const FQuat BackRotation(FVector::XAxisVector, -Pitch);
    const FVector BackCenter(Width * 0.5f, Depth - SlopeCenterY, SlopeCenterZ);
    AddMeshInstance(RoofISM, BackCenter + BackRotation.GetUpVector() * HalfThickness, BackRotation, Scale);

    // Gable ends, approximated with stacked wall strips that narrow towards the ridge.
    const float RidgeHeight = HalfDepth * FMath::Tan(Pitch);
    const float StepHeight = RidgeHeight / MCO_GriffinHouse::GableSteps;
    const float Flip = bFlipWallFacing ? 180.f : 0.f;

    for (int32 Step = 0; Step < MCO_GriffinHouse::GableSteps; ++Step)
    {
        const float StripWidth = Depth * (1.f - (Step + 0.5f) / MCO_GriffinHouse::GableSteps);
        const FVector StripScale(StripWidth / WallMeshSize.X, 1.f, StepHeight / WallMeshSize.Z);
        const float Z = RoofBase + StepHeight * (Step + 0.5f);

        AddMeshInstance(WallISM, FVector(0.f, HalfDepth, Z), FQuat(FRotator(0.f, -90.f + Flip, 0.f)), StripScale);
        AddMeshInstance(WallISM, FVector(Width, HalfDepth, Z), FQuat(FRotator(0.f, 90.f + Flip, 0.f)), StripScale);
    }
}

void AMCO_GriffinHouse::BuildCurb()
{
    const float CurbHeight = MCO_GriffinHouse::GetMeshSize(CurbISM, FVector(300.f, 50.f, 50.f)).Z;

    // Runs one module past the house and garage on both sides.
    for (int32 X = -1; X < HouseWidth + GarageWidth + 1; ++X)
        AddMeshInstance(CurbISM, FVector((X + 0.5f) * ModuleSize, -CurbDistance, CurbHeight * 0.5f), FQuat::Identity);
}

void AMCO_GriffinHouse::AddMeshInstance(UInstancedStaticMeshComponent* ISM, const FVector& Center, const FQuat& Rotation, const FVector& Scale)
{
    if (!ISM || !ISM->GetStaticMesh())
        return;

    const FVector MeshCenter = ISM->GetStaticMesh()->GetBoundingBox().GetCenter();
    const FVector Location = Center - Rotation.RotateVector(MeshCenter * Scale);
    ISM->AddInstance(FTransform(Rotation, Location, Scale));
}

void AMCO_GriffinHouse::AddWallActor(TSubclassOf<AActor> Class, const FVector& BottomCenter, float Yaw)
{
    if (!Class)
        return;

    UChildActorComponent* ChildComp = NewObject<UChildActorComponent>(this);
    ChildComp->CreationMethod = EComponentCreationMethod::UserConstructionScript;
    ChildComp->SetupAttachment(SceneRootComp);
    ChildComp->SetRelativeLocationAndRotation(BottomCenter, FRotator(0.f, Yaw, 0.f));
    ChildComp->SetChildActorClass(Class);
    ChildComp->RegisterComponent();

    AActor* Child = ChildComp->GetChildActor();
    if (!Child)
        return;

    // Line the module up using its biggest mesh (the wall), so we don't depend on how the blueprint is pivoted.
    const FTransform HouseTransform = GetActorTransform();
    auto GetWallBox = [Child, &HouseTransform]()
    {
        FBox WallBox(ForceInit);
        Child->ForEachComponent<UStaticMeshComponent>(false, [&](const UStaticMeshComponent* Mesh)
        {
            if (!Mesh->GetStaticMesh() || Mesh->IsA<UInstancedStaticMeshComponent>())
                return;

            const FBox Box = Mesh->CalcBounds(Mesh->GetComponentTransform().GetRelativeTransform(HouseTransform)).GetBox();
            if (!WallBox.IsValid || Box.GetVolume() > WallBox.GetVolume())
                WallBox = Box;
        });
        return WallBox;
    };

    FBox WallBox = GetWallBox();
    if (!WallBox.IsValid)
        return;

    // Turn the module if its wall isn't running along the run direction.
    const bool bRunAlongX = FMath::Abs(FMath::Cos(FMath::DegreesToRadians(Yaw))) > 0.5f;
    const FVector WallSize = WallBox.GetSize();
    if (bRunAlongX != (WallSize.X >= WallSize.Y))
    {
        ChildComp->AddRelativeRotation(FRotator(0.f, 90.f, 0.f));
        WallBox = GetWallBox();
    }

    const FVector WallCenter = WallBox.GetCenter();
    ChildComp->AddRelativeLocation(FVector(
        BottomCenter.X - WallCenter.X,
        BottomCenter.Y - WallCenter.Y,
        BottomCenter.Z - WallBox.Min.Z));
}

FVector AMCO_GriffinHouse::CellCenter(int32 X, int32 Y, float Z)
{
    return FVector((X + 0.5f) * ModuleSize, (Y + 0.5f) * ModuleSize, Z);
}
