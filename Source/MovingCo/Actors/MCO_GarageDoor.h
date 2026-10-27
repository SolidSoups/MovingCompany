#pragma once

#include "CoreMinimal.h"
#include "MCO_Door.h"
#include "GameFramework/Actor.h"
#include "MCO_GarageDoor.generated.h"

UCLASS()
class MOVINGCO_API AMCO_GarageDoor : public AMCO_Door
{
	GENERATED_BODY()

public:
	AMCO_GarageDoor(const FObjectInitializer& ObjectInitializer);

};
