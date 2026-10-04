#include "MCO_DebrisActor.h"

AMCO_DebrisActor::AMCO_DebrisActor()
{
	PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetCollisionProfileName(FName("PhysicsActor"));
    RootComponent = Mesh;
}

void AMCO_DebrisActor::Init(UStaticMesh *InMesh, UMaterialInterface *Material){
    Mesh->SetStaticMesh(InMesh);
    Mesh->SetMaterial(0, Material);
    Mesh->SetSimulatePhysics(true);
}

void AMCO_DebrisActor::Launch(const FVector &Impulse, const FVector &Location){
    Mesh->AddImpulseAtLocation(Impulse, Location);
}

