#include "MCO_FurnitureBase.h"

#include "Components/StaticMeshComponent.h"

AMCO_FurnitureBase::AMCO_FurnitureBase()
{
	PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetCollisionProfileName(FName("PhysicsActor"));
    Mesh->SetSimulatePhysics(true);
    Mesh->BodyInstance.SetMassOverride(500.0f);
    RootComponent = Mesh;
}

void AMCO_FurnitureBase::BeginPlay()
{
	Super::BeginPlay();
}

void AMCO_FurnitureBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
