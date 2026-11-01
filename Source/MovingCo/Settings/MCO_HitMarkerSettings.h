#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MCO_HitMarkerSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Hit Markers"))
class MOVINGCO_API UMCO_HitMarkerSettings : public UDeveloperSettings {
    GENERATED_BODY()

public:
    UPROPERTY(Config, EditAnywhere, Category="Hit Markers")
    TSoftObjectPtr<class UNiagaraSystem> HitMarkerSystem;
};
