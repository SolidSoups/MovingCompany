#include "MCO_FurnitureBase.h"

#include "Components/StaticMeshComponent.h"
#include "Player/MovingCoCharacter.h"
#include "Components/MCO_ItemValueComponent.h"

AMCO_FurnitureBase::AMCO_FurnitureBase()
{
	PrimaryActorTick.bCanEverTick = false;
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

void AMCO_FurnitureBase::BeginPlay()
{
	Super::BeginPlay();
    SetPhysicsReplicationMode(EPhysicsReplicationMode::PredictiveInterpolation);
    
    if(HasAuthority()){
        Mesh->SetSimulatePhysics(true);
        Mesh->SetNotifyRigidBodyCollision(true);
        Mesh->OnComponentHit.AddDynamic(this, &AMCO_FurnitureBase::OnMeshHit);

        LastImpactTime = 0.0f;
    }
}
void AMCO_FurnitureBase::OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // Ignore collisions with player
    if(Cast<AMovingCoCharacter>(OtherActor))
        return;

    // Ensure that impacts aren't recorded TOO often
    const float Now = GetWorld()->GetTimeSeconds();
    if(Now - LastImpactTime < ImpactDelay)
        return;
    LastImpactTime = Now;

    // Calculate impact speed
    const float ImpactSize = NormalImpulse.Size();
    const float ImpactSpeed = ImpactSize / FMath::Max(Mesh->GetMass(), 1.0f); 

    // Ensure impact is violent enough
    if(ImpactSpeed < MinImpactSpeed)
        return;

    // We need the location and intensity for this sound to play,
    // and so we can differentiate between small and large impacts
    const FVector Location = Hit.ImpactPoint.IsZero() 
        ? GetActorLocation() 
        : FVector(Hit.ImpactPoint);
    const float DamageIntensity = FMath::GetMappedRangeValueClamped(
        FVector2D(MinImpactSpeed, MaxImpactSpeed),
        FVector2D(0.0f, 1.0f),
        ImpactSpeed
    );

    // NetMulticast here!
    Multicast_PlayImpactFX(Location, NormalImpulse, DamageIntensity);
   
    // Apply damage to the furniture here  
    // Please make sure only to bind to the dynamic events on the blueprint side.
    ItemValueComponent->DamageValuable(DamageIntensity * 100.f, Location);
}

void AMCO_FurnitureBase::Multicast_PlayImpactFX_Implementation(FVector_NetQuantize10 Location,
                                                               FVector_NetQuantize10 NormalImpulse,
                                                               float IntensityNorm)
{
    if(GetNetMode() == NM_DedicatedServer)
        return;

    PlayImpactFX(FVector(Location), FVector(NormalImpulse), IntensityNorm);
}
