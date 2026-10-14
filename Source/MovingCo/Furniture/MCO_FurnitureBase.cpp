#include "MCO_FurnitureBase.h"

#include "Components/StaticMeshComponent.h"

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
    RootComponent = Mesh;
}

void AMCO_FurnitureBase::BeginPlay()
{
	Super::BeginPlay();
    SetPhysicsReplicationMode(EPhysicsReplicationMode::PredictiveInterpolation);
    Mesh->SetSimulatePhysics(HasAuthority());
}

void AMCO_FurnitureBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
