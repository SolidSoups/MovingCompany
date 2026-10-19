#include "Building/MCO_CityBuilding.h"
#include "Components/ChildActorComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

namespace MCO_CityBuilding
{
    // The stairwell sits in the left column of cells, against the back wall.
    constexpr int32 StairsCellX = 0;
    constexpr int32 StairsCellCount = 2;

    FVector GetMeshSize(const UInstancedStaticMeshComponent* ISM, const FVector& Fallback)
    {
        const UStaticMesh* Mesh = ISM ? ISM->GetStaticMesh() : nullptr;
        return Mesh ? Mesh->GetBoundingBox().GetSize() : Fallback;
    }
}

AMCO_CityBuilding::AMCO_CityBuilding(const FObjectInitializer& ObjectInitializer)
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
    StairsISM = CreateISM(TEXT("Stairs"), TEXT("/Game/Art/Modular_Pieces/City/SM_City_Stairs_A.SM_City_Stairs_A"));
    TrimISM = CreateISM(TEXT("Trims"), TEXT("/Game/Art/Modular_Pieces/City/SM_Wall_Trim_Vertical_A.SM_Wall_Trim_Vertical_A"));
    SidewalkISM = CreateISM(TEXT("Sidewalk"), TEXT("/Game/Art/Modular_Pieces/City/SM_City_Floor_3x3.SM_City_Floor_3x3"));
    CurbISM = CreateISM(TEXT("Curbs"), TEXT("/Game/Art/Modular_Pieces/City/SM_City_Curb_3m_A.SM_City_Curb_3m_A"));

    static ConstructorHelpers::FClassFinder<AActor> DoorClassFinder(TEXT("/Game/Blueprints/Actors/Door/MCO_BP_Door"));
    static ConstructorHelpers::FClassFinder<AActor> WindowClassFinder(TEXT("/Game/Blueprints/Actors/Door/MCO_BP_Window"));
    static ConstructorHelpers::FClassFinder<AActor> GarageDoorClassFinder(TEXT("/Game/Blueprints/Actors/Door/MCO_BP_GarageDoor"));
    DoorWallClass = DoorClassFinder.Class;
    WindowWallClass = WindowClassFinder.Class;
    GarageDoorWallClass = GarageDoorClassFinder.Class;
}

void AMCO_CityBuilding::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    for (UInstancedStaticMeshComponent* ISM : { WallISM, FloorISM, CeilingISM, StairsISM, TrimISM, SidewalkISM, CurbISM })
        ISM->ClearInstances();

    const float Flip = bFlipWallFacing ? 180.f : 0.f;
    const float BuildingWidth = Width * ModuleSize;
    const float BuildingDepth = Depth * ModuleSize;

    // Wall yaw is chosen so the exterior faces outwards (-Y at yaw 0).
    for (int32 Story = 0; Story < Stories; ++Story)
    {
        const bool bGround = Story == 0;
        const float Z = Story * WallHeight;
        BuildWallRun(bGround ? FrontGround : FrontUpper, FVector(0.f, 0.f, Z), FVector::XAxisVector, Flip, Width);
        BuildWallRun(bGround ? BackGround : BackUpper, FVector(0.f, BuildingDepth, Z), FVector::XAxisVector, 180.f + Flip, Width);
        BuildWallRun(bGround ? LeftGround : LeftUpper, FVector(0.f, 0.f, Z), FVector::YAxisVector, -90.f + Flip, Depth);
        BuildWallRun(bGround ? RightGround : RightUpper, FVector(BuildingWidth, 0.f, Z), FVector::YAxisVector, 90.f + Flip, Depth);
    }

    BuildCornerTrims();
    BuildFloors();
    BuildStairs();
    BuildParapet();

    if (bBuildSidewalk)
        BuildSidewalk();
}

void AMCO_CityBuilding::BuildWallRun(const FString& Pattern, const FVector& Start, const FVector& Direction, float Yaw, int32 MaxModules)
{
    if (Pattern.IsEmpty())
        return;

    const FQuat Rotation(FRotator(0.f, Yaw, 0.f));

    int32 Slot = 0;
    for (int32 Index = 0; Slot < MaxModules; ++Index)
    {
        TCHAR Char = FChar::ToUpper(Pattern[Index % Pattern.Len()]);
        if (Char == TEXT('G') && Slot + 2 > MaxModules)
            Char = TEXT('S');

        const int32 Modules = Char == TEXT('G') ? 2 : 1;
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

void AMCO_CityBuilding::BuildCornerTrims()
{
    const float BuildingWidth = Width * ModuleSize;
    const float BuildingDepth = Depth * ModuleSize;

    for (int32 Story = 0; Story < Stories; ++Story)
    {
        const float Z = (Story + 0.5f) * WallHeight;
        for (const FVector2D& Corner : { FVector2D(0.f, 0.f), FVector2D(BuildingWidth, 0.f), FVector2D(0.f, BuildingDepth), FVector2D(BuildingWidth, BuildingDepth) })
            AddMeshInstance(TrimISM, FVector(Corner, Z), FQuat::Identity);
    }
}

void AMCO_CityBuilding::BuildFloors()
{
    const float FloorThickness = MCO_CityBuilding::GetMeshSize(FloorISM, FVector(300.f, 300.f, 30.f)).Z;
    const float CeilingThickness = MCO_CityBuilding::GetMeshSize(CeilingISM, FVector(300.f, 300.f, 50.f)).Z;

    for (int32 X = 0; X < Width; ++X)
    {
        for (int32 Y = 0; Y < Depth; ++Y)
        {
            // Ground floor top at Z = 0, every upper slab sits on the walls of the story below.
            AddMeshInstance(FloorISM, CellCenter(X, Y, -FloorThickness * 0.5f), FQuat::Identity);
            if (!IsStairsCell(X, Y))
            {
                for (int32 Story = 1; Story < Stories; ++Story)
                    AddMeshInstance(FloorISM, CellCenter(X, Y, Story * WallHeight + FloorThickness * 0.5f), FQuat::Identity);
            }

            // Flat roof.
            AddMeshInstance(CeilingISM, CellCenter(X, Y, Stories * WallHeight + CeilingThickness * 0.5f), FQuat::Identity);
        }
    }
}

void AMCO_CityBuilding::BuildStairs()
{
    if (!StairsISM->GetStaticMesh())
        return;

    const FVector StairsSize = StairsISM->GetStaticMesh()->GetBoundingBox().GetSize();
    const float WallHalfThickness = MCO_CityBuilding::GetMeshSize(WallISM, FVector(300.f, 50.f, 400.f)).Y * 0.5f;
    const FQuat Rotation(FRotator(0.f, StairsYaw, 0.f));
    const FVector RunDirection = FVector::YAxisVector;

    // Two flights back to back against the left wall, ending at the back wall, repeated up to the top story.
    const FVector RunMiddle(
        MCO_CityBuilding::StairsCellX * ModuleSize + WallHalfThickness + StairsSize.X * 0.5f,
        Depth * ModuleSize - WallHalfThickness - StairsSize.Y,
        0.f);

    for (int32 Story = 0; Story < Stories - 1; ++Story)
    {
        for (int32 Flight = 0; Flight < 2; ++Flight)
        {
            FVector Center = RunMiddle + RunDirection * StairsSize.Y * (Flight - 0.5f);
            Center.Z = Story * WallHeight + StairsSize.Z * (Flight + 0.5f);
            AddMeshInstance(StairsISM, Center, Rotation);
        }
    }
}

void AMCO_CityBuilding::BuildParapet()
{
    if (ParapetHeight <= 0.f)
        return;

    const FVector WallMeshSize = MCO_CityBuilding::GetMeshSize(WallISM, FVector(300.f, 50.f, 400.f));
    const FVector TrimMeshSize = MCO_CityBuilding::GetMeshSize(TrimISM, FVector(75.f, 75.f, 400.f));
    const float BuildingWidth = Width * ModuleSize;
    const float BuildingDepth = Depth * ModuleSize;
    const float Z = Stories * WallHeight + ParapetHeight * 0.5f;
    const float Flip = bFlipWallFacing ? 180.f : 0.f;
    const FVector WallScale(1.f, 1.f, ParapetHeight / WallMeshSize.Z);

    for (int32 X = 0; X < Width; ++X)
    {
        const float CenterX = (X + 0.5f) * ModuleSize;
        AddMeshInstance(WallISM, FVector(CenterX, 0.f, Z), FQuat(FRotator(0.f, Flip, 0.f)), WallScale);
        AddMeshInstance(WallISM, FVector(CenterX, BuildingDepth, Z), FQuat(FRotator(0.f, 180.f + Flip, 0.f)), WallScale);
    }

    for (int32 Y = 0; Y < Depth; ++Y)
    {
        const float CenterY = (Y + 0.5f) * ModuleSize;
        AddMeshInstance(WallISM, FVector(0.f, CenterY, Z), FQuat(FRotator(0.f, -90.f + Flip, 0.f)), WallScale);
        AddMeshInstance(WallISM, FVector(BuildingWidth, CenterY, Z), FQuat(FRotator(0.f, 90.f + Flip, 0.f)), WallScale);
    }

    const FVector TrimScale(1.f, 1.f, ParapetHeight / TrimMeshSize.Z);
    for (const FVector2D& Corner : { FVector2D(0.f, 0.f), FVector2D(BuildingWidth, 0.f), FVector2D(0.f, BuildingDepth), FVector2D(BuildingWidth, BuildingDepth) })
        AddMeshInstance(TrimISM, FVector(Corner, Z), FQuat::Identity, TrimScale);
}

void AMCO_CityBuilding::BuildSidewalk()
{
    const float SidewalkThickness = MCO_CityBuilding::GetMeshSize(SidewalkISM, FVector(300.f, 300.f, 30.f)).Z;
    const float CurbHeight = MCO_CityBuilding::GetMeshSize(CurbISM, FVector(300.f, 50.f, 50.f)).Z;
    const int32 Rows = FMath::CeilToInt(CurbDistance / ModuleSize);

    // Runs one module past the building on both sides so neighbouring buildings connect.
    for (int32 X = -1; X < Width + 1; ++X)
    {
        for (int32 Row = 1; Row <= Rows; ++Row)
            AddMeshInstance(SidewalkISM, CellCenter(X, -Row, -SidewalkThickness * 0.5f), FQuat::Identity);

        AddMeshInstance(CurbISM, FVector((X + 0.5f) * ModuleSize, -CurbDistance, CurbHeight * 0.5f), FQuat::Identity);
    }
}

bool AMCO_CityBuilding::IsStairsCell(int32 X, int32 Y) const
{
    return Stories > 1 && X == MCO_CityBuilding::StairsCellX && Y >= Depth - MCO_CityBuilding::StairsCellCount;
}

void AMCO_CityBuilding::AddMeshInstance(UInstancedStaticMeshComponent* ISM, const FVector& Center, const FQuat& Rotation, const FVector& Scale)
{
    if (!ISM || !ISM->GetStaticMesh())
        return;

    const FVector MeshCenter = ISM->GetStaticMesh()->GetBoundingBox().GetCenter();
    const FVector Location = Center - Rotation.RotateVector(MeshCenter * Scale);
    ISM->AddInstance(FTransform(Rotation, Location, Scale));
}

void AMCO_CityBuilding::AddWallActor(TSubclassOf<AActor> Class, const FVector& BottomCenter, float Yaw)
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
    const FTransform BuildingTransform = GetActorTransform();
    auto GetWallBox = [Child, &BuildingTransform]()
    {
        FBox WallBox(ForceInit);
        Child->ForEachComponent<UStaticMeshComponent>(false, [&](const UStaticMeshComponent* Mesh)
        {
            if (!Mesh->GetStaticMesh() || Mesh->IsA<UInstancedStaticMeshComponent>())
                return;

            const FBox Box = Mesh->CalcBounds(Mesh->GetComponentTransform().GetRelativeTransform(BuildingTransform)).GetBox();
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

FVector AMCO_CityBuilding::CellCenter(int32 X, int32 Y, float Z)
{
    return FVector((X + 0.5f) * ModuleSize, (Y + 0.5f) * ModuleSize, Z);
}
