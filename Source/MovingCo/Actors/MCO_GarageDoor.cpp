#include "MCO_GarageDoor.h"
#include "GeometryCacheComponent.h"

AMCO_GarageDoor::AMCO_GarageDoor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UGeometryCacheComponent>(DoorSceneCompName))
{
}
