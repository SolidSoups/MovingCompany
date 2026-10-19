#include "MCO_DeliveryZone.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "MCO_FurnitureBase.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

AMCO_DeliveryZone::AMCO_DeliveryZone()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    bAlwaysRelevant = true;

    Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("Zone"));
    Zone->InitBoxExtent(FVector(200.f, 200.f, 100.f));
    Zone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Zone->SetHiddenInGame(false);
    Zone->ShapeColor = FColor::Green;
    Zone->SetLineThickness(4.f);
    RootComponent = Zone;

    Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
    Label->SetupAttachment(Zone);
    Label->SetRelativeLocation(FVector(0.f, 0.f, 160.f));
    Label->SetHorizontalAlignment(EHTA_Center);
    Label->SetVerticalAlignment(EVRTA_TextCenter);
    Label->SetWorldSize(40.f);
    Label->SetTextRenderColor(FColor::Green);
}

void AMCO_DeliveryZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMCO_DeliveryZone, DeliveredCount);
    DOREPLIFETIME(AMCO_DeliveryZone, RequiredCount);
}

void AMCO_DeliveryZone::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        // Polling instead of overlap events, furniture meshes don't generate them by default
        GetWorldTimerManager().SetTimer(RecountTimer, this, &AMCO_DeliveryZone::Recount, RecountInterval, true);
        Recount();
    }

    UpdateLabel();
}

void AMCO_DeliveryZone::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Keep the label facing the local player
    if (const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0))
    {
        const FVector ToCamera = Camera->GetCameraLocation() - Label->GetComponentLocation();
        Label->SetWorldRotation(FRotator(0.f, ToCamera.Rotation().Yaw, 0.f));
    }
}

void AMCO_DeliveryZone::Recount()
{
    const FTransform& ZoneTransform = Zone->GetComponentTransform();
    const FVector Extent = Zone->GetUnscaledBoxExtent();

    int32 Required = 0;
    int32 Delivered = 0;

    for (TActorIterator<AMCO_FurnitureBase> It(GetWorld()); It; ++It)
    {
        if (!It->bIsScorable)
            continue;

        ++Required;

        // Has to be set down with its center inside the zone
        const FVector Local = ZoneTransform.InverseTransformPosition(It->GetComponentsBoundingBox().GetCenter());
        const bool bInside = FMath::Abs(Local.X) <= Extent.X and FMath::Abs(Local.Y) <= Extent.Y and
                             FMath::Abs(Local.Z) <= Extent.Z;
        if (bInside and !It->IsHeld())
            ++Delivered;
    }

    if (Required == RequiredCount and Delivered == DeliveredCount)
        return;

    RequiredCount = Required;
    DeliveredCount = Delivered;
    UpdateLabel();

    if (!bAllDelivered and RequiredCount > 0 and DeliveredCount == RequiredCount)
    {
        bAllDelivered = true;
        OnAllFurnitureDelivered.Broadcast();
        Multicast_AnnounceDelivered();
    }
}

void AMCO_DeliveryZone::UpdateLabel()
{
    Label->SetText(FText::FromString(FString::Printf(TEXT("Delivered %d / %d"), DeliveredCount, RequiredCount)));
}

void AMCO_DeliveryZone::Multicast_AnnounceDelivered_Implementation()
{
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("All furniture delivered! Job complete."));
}
