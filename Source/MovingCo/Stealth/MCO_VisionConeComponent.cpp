#include "MCO_VisionConeComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

UMCO_VisionConeComponent::UMCO_VisionConeComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);

    SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetCanEverAffectNavigation(false);
    SetCastShadow(false);
    bUseAsyncCooking = true;

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMaterial(
        TEXT("/Engine/EngineDebugMaterials/M_SimpleUnlitTranslucent.M_SimpleUnlitTranslucent"));
    if (DefaultMaterial.Succeeded())
        ConeMaterial = DefaultMaterial.Object;
}

void UMCO_VisionConeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMCO_VisionConeComponent, bConeActive);
    DOREPLIFETIME(UMCO_VisionConeComponent, AlertLevel);
}

void UMCO_VisionConeComponent::BeginPlay()
{
    Super::BeginPlay();

    if (ConeMaterial)
    {
        ConeMID = UMaterialInstanceDynamic::Create(ConeMaterial, this);
        SetMaterial(0, ConeMID);
    }

    // Fan triangles never change, only the vertex positions do.
    // Winding is (center, next, current) so the face points up in Unreal's left-handed space
    const int32 NumRays = FMath::Max(Segments, 2) + 1;
    Triangles.Reset();
    for (int32 i = 1; i < NumRays; ++i)
    {
        Triangles.Add(0);
        Triangles.Add(i + 1);
        Triangles.Add(i);
    }
    Normals.Init(FVector::UpVector, NumRays + 1);

    if (GetOwner()->HasAuthority())
        bConeActive = bStartActive;

    UpdateVisuals();
}

void UMCO_VisionConeComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bConeActive and GetNetMode() != NM_DedicatedServer)
        RebuildMesh();
}

FVector UMCO_VisionConeComponent::GetEyeLocation() const
{
    if (const APawn* Pawn = Cast<APawn>(GetOwner()))
        return Pawn->GetPawnViewLocation();

    return GetComponentLocation() + FVector(0.f, 0.f, 150.f);
}

bool UMCO_VisionConeComponent::CanSeePoint(const FVector& Point, const TArray<const AActor*>& IgnoredActors) const
{
    if (!bConeActive)
        return false;

    const FVector Eye = GetEyeLocation();
    const FVector ToPoint = Point - Eye;
    if (ToPoint.SizeSquared() > FMath::Square(ViewDistance))
        return false;

    // The cone is flat on the floor, so only the horizontal angle matters
    const FVector FlatDir = ToPoint.GetSafeNormal2D();
    const FVector Forward = GetForwardVector().GetSafeNormal2D();
    if (!FlatDir.IsNearlyZero() and
        FVector::DotProduct(FlatDir, Forward) < FMath::Cos(FMath::DegreesToRadians(ViewHalfAngle)))
    {
        return false;
    }

    FCollisionQueryParams Params(SCENE_QUERY_STAT(VisionConeCheck), false, GetOwner());
    Params.AddIgnoredActors(IgnoredActors);

    FHitResult Hit;
    if (!GetWorld()->LineTraceSingleByChannel(Hit, Eye, Point, ECC_Visibility, Params))
        return true;

    // Points on a surface (e.g. furniture scraping a wall) would otherwise be blocked by that same surface
    return FVector::DistSquared(Hit.ImpactPoint, Point) < FMath::Square(30.f);
}

bool UMCO_VisionConeComponent::CanSeeActor(const AActor* Target) const
{
    if (!Target)
        return false;

    return CanSeePoint(Target->GetActorLocation(), {Target});
}

void UMCO_VisionConeComponent::SetConeActive(bool bInActive)
{
    bConeActive = bInActive;
    UpdateVisuals();
}

void UMCO_VisionConeComponent::SetAlertLevel(EMCO_AlertLevel InLevel)
{
    if (AlertLevel == InLevel)
        return;

    AlertLevel = InLevel;
    UpdateVisuals();
}

void UMCO_VisionConeComponent::UpdateVisuals()
{
    SetVisibility(bConeActive);

    if (!ConeMID)
        return;

    float Opacity = AlertOpacity.X;
    if (AlertLevel == EMCO_AlertLevel::Suspicious)
        Opacity = AlertOpacity.Y;
    else if (AlertLevel == EMCO_AlertLevel::Alerted)
        Opacity = AlertOpacity.Z;

    FLinearColor Color = ConeColor;
    Color.A = Opacity;
    ConeMID->SetVectorParameterValue(TEXT("Color"), Color);
    ConeMID->SetScalarParameterValue(TEXT("Opacity"), Opacity);
}

void UMCO_VisionConeComponent::RebuildMesh()
{
    const int32 NumRays = FMath::Max(Segments, 2) + 1;
    const FVector Eye = GetEyeLocation();
    const FVector Forward = GetForwardVector().GetSafeNormal2D();
    const FTransform& ComponentTransform = GetComponentTransform();

    FCollisionQueryParams Params(SCENE_QUERY_STAT(VisionConeMesh), false, GetOwner());

    Vertices.Reset(NumRays + 1);
    Vertices.Add(FVector::ZeroVector);

    for (int32 i = 0; i < NumRays; ++i)
    {
        const float Angle = -ViewHalfAngle + 2.f * ViewHalfAngle * i / (NumRays - 1);
        const FVector Dir = Forward.RotateAngleAxis(Angle, FVector::UpVector);

        // Trace at eye height so the fan is cut off by walls but not by the floor
        float Distance = ViewDistance;
        FHitResult Hit;
        if (GetWorld()->LineTraceSingleByChannel(Hit, Eye, Eye + Dir * ViewDistance, ECC_Visibility, Params))
            Distance = Hit.Distance;

        Vertices.Add(ComponentTransform.InverseTransformPosition(GetComponentLocation() + Dir * Distance));
    }

    if (!bSectionCreated)
    {
        CreateMeshSection(0, Vertices, Triangles, Normals, {}, {}, {}, false);
        bSectionCreated = true;
    }
    else
    {
        UpdateMeshSection(0, Vertices, Normals, {}, {}, {});
    }
}
