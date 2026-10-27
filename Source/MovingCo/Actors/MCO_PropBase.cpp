#include "MCO_PropBase.h"

#include "Components/StaticMeshComponent.h"
#include "Player/MovingCoCharacter.h"
#include "Components/MCO_ItemValueComponent.h"

AMCO_PropBase::AMCO_PropBase()
{
	PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    bReplicates = true;
    SetReplicateMovement(true);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetCollisionProfileName(FName("PhysicsActor"));
    Mesh->SetNotifyRigidBodyCollision(true);
    Mesh->BodyInstance.SetMassOverride(500.0f);
    RootComponent = Mesh;

    ItemValueComponent = CreateDefaultSubobject<UMCO_ItemValueComponent>(TEXT("ItemValueComponent"));
}

void AMCO_PropBase::BeginPlay()
{
	Super::BeginPlay();
    SetPhysicsReplicationMode(EPhysicsReplicationMode::PredictiveInterpolation);
    
    if(HasAuthority()){
        Mesh->SetSimulatePhysics(true);
        Mesh->SetNotifyRigidBodyCollision(true);
        Mesh->OnComponentHit.AddDynamic(this, &AMCO_PropBase::OnMeshHit);

        LastImpactTime = 0.0f;

        SetActorTickEnabled(true);
    }
}

void AMCO_PropBase::Tick(float DeltaTime)
{
    // Tick only runs on server to track furniture velocity
    LastVelocity = Mesh->GetPhysicsLinearVelocity();
}

void AMCO_PropBase::OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // Ensure that impacts aren't recorded TOO often
    const float Now = GetWorld()->GetTimeSeconds();
    if(Now - LastImpactTime < ImpactDelay)
        return;
    LastImpactTime = Now;

    // Resolve the velocity of the other object
    FVector OtherVelocity = FVector::ZeroVector;
    if(auto* OtherFurniture = Cast<AMCO_PropBase>(OtherActor)){
        OtherVelocity = OtherFurniture->GetLastVelocity();
    }
    else if(auto* OtherPlayer = Cast<AMovingCoCharacter>(OtherActor)){
        OtherVelocity = OtherPlayer->GetVelocity();
    }

    // Project the summed velocity onto the impact normal. This gives
    // us the accumulated speed of the impact.
    FVector SumVelocity = LastVelocity - OtherVelocity;
    FVector ProjectedVelocity = SumVelocity.ProjectOnTo(Hit.ImpactNormal);
    const float ImpactVelocity = ProjectedVelocity.Size();

    // Ensure impact is violent enough
    if(ImpactVelocity < MinImpactSpeed)
        return;

    // We need the location and intensity for this sound to play,
    // and so we can differentiate between small and large impacts
    const FVector Location = Hit.ImpactPoint.IsZero() 
        ? GetActorLocation() 
        : FVector(Hit.ImpactPoint);
    const float DamageIntensity = FMath::GetMappedRangeValueClamped(
        FVector2D(MinImpactSpeed, MaxImpactSpeed),
        FVector2D(0.0f, 1.0f),
        ImpactVelocity
    );

    // NetMulticast here!
    Multicast_PlayImpactFX(Location, NormalImpulse, DamageIntensity);
   
    // Apply damage to the furniture here  
    // Please make sure only to bind to the dynamic events on the blueprint side.
    ItemValueComponent->DamageValuable(DamageIntensity * 100.f, Location);
}

void AMCO_PropBase::Multicast_PlayImpactFX_Implementation(FVector_NetQuantize10 Location,
                                                               FVector_NetQuantize10 NormalImpulse,
                                                               float IntensityNorm)
{
    if(GetNetMode() == NM_DedicatedServer)
        return;

    PlayImpactFX(FVector(Location), FVector(NormalImpulse), IntensityNorm);
}
