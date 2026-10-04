// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MovingCoGameMode.generated.h"

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class AMovingCoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMovingCoGameMode();
};



