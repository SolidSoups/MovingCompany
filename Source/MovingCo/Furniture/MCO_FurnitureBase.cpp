#include "MCO_FurnitureBase.h"

#include "Audio/MCO_ProceduralThud.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundWaveProcedural.h"
#include "Stealth/MCO_StealthSubsystem.h"
#include "TimerManager.h"

AMCO_FurnitureBase::AMCO_FurnitureBase()
{
	PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetCollisionProfileName(FName("PhysicsActor"));
    // Mesh->SetIsReplicated(true);
    Mesh->BodyInstance.SetMassOverride(500.0f);
    Mesh->SetNotifyRigidBodyCollision(true);
    RootComponent = Mesh;
}

void AMCO_FurnitureBase::BeginPlay()
{
	Super::BeginPlay();
    SetPhysicsReplicationMode(EPhysicsReplicationMode::PredictiveInterpolation);
    Mesh->SetSimulatePhysics(HasAuthority());

    // Physics only runs on the server, so that's where hits come from
    if (HasAuthority())
        Mesh->OnComponentHit.AddDynamic(this, &AMCO_FurnitureBase::OnMeshHit);
}

void AMCO_FurnitureBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMCO_FurnitureBase::AddHolder(APawn* Pawn)
{
    Holders.AddUnique(Pawn);
}

void AMCO_FurnitureBase::RemoveHolder(APawn* Pawn)
{
    Holders.Remove(Pawn);
    LastHolder = Pawn;
    LastReleaseTime = GetWorld()->GetTimeSeconds();
}

bool AMCO_FurnitureBase::IsHeld() const
{
    return Holders.ContainsByPredicate([](const TWeakObjectPtr<APawn>& Holder) { return Holder.IsValid(); });
}

APawn* AMCO_FurnitureBase::GetResponsiblePawn() const
{
    for (const TWeakObjectPtr<APawn>& Holder : Holders)
    {
        if (Holder.IsValid())
            return Holder.Get();
    }

    if (GetWorld()->GetTimeSeconds() - LastReleaseTime <= BlameTime)
        return LastHolder.Get();

    return nullptr;
}

void AMCO_FurnitureBase::OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // Whoever is carrying us is constantly touching us, that's not a bump
    if (Cast<APawn>(OtherActor))
        return;

    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastBumpTime < BumpCooldown)
        return;

    // Velocity change along the normal: resting or sliding on the floor stays well below this, actual hits don't
    const float ImpactSpeed = NormalImpulse.Size() / FMath::Max(Mesh->GetMass(), 1.f);
    if (ImpactSpeed < MinBumpSpeed)
        return;

    LastBumpTime = Now;

    const float Intensity = FMath::GetMappedRangeValueClamped(FVector2D(MinBumpSpeed, LoudBumpSpeed),
                                                              FVector2D(0.f, 1.f), ImpactSpeed);
    const FVector Location = Hit.ImpactPoint.IsZero() ? GetActorLocation() : FVector(Hit.ImpactPoint);

    Multicast_PlayBump(Location, Intensity);

    if (UMCO_StealthSubsystem* Stealth = GetWorld()->GetSubsystem<UMCO_StealthSubsystem>())
        Stealth->ReportBump(this, Location, FMath::Lerp(0.5f, 1.f, Intensity), GetResponsiblePawn());
}

void AMCO_FurnitureBase::Multicast_PlayBump_Implementation(FVector_NetQuantize Location, float Intensity)
{
    if (GetNetMode() == NM_DedicatedServer)
        return;

    if (BumpSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BumpSound, Location, FMath::Lerp(0.4f, 1.f, Intensity),
                                              FMath::FRandRange(0.9f, 1.1f), 0.f, GetBumpAttenuation());
        return;
    }

    UAudioComponent* Audio = UGameplayStatics::SpawnSoundAtLocation(
        this, MCO::Audio::CreateThud(Intensity), Location, FRotator::ZeroRotator, 1.f, 1.f, 0.f,
        GetBumpAttenuation());
    if (!Audio)
        return;

    // Procedural sounds never finish by themselves
    FTimerHandle StopTimer;
    GetWorldTimerManager().SetTimer(StopTimer, FTimerDelegate::CreateWeakLambda(Audio, [Audio]() { Audio->Stop(); }),
                                    MCO::Audio::ThudDuration + 0.05f, false);
}

USoundAttenuation* AMCO_FurnitureBase::GetBumpAttenuation()
{
    if (BumpAttenuation)
        return BumpAttenuation;

    if (!DefaultBumpAttenuation)
    {
        DefaultBumpAttenuation = NewObject<USoundAttenuation>(this);
        FSoundAttenuationSettings& Settings = DefaultBumpAttenuation->Attenuation;
        Settings.bAttenuate = true;
        Settings.bSpatialize = true;
        Settings.AttenuationShape = EAttenuationShape::Sphere;
        Settings.AttenuationShapeExtents = FVector(300.f, 0.f, 0.f);
        Settings.FalloffDistance = 2500.f;
    }
    return DefaultBumpAttenuation;
}
