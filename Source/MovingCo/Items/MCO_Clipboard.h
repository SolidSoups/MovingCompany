#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MCO_Clipboard.generated.h"

UCLASS()
class MOVINGCO_API AMCO_Clipboard : public AActor
{
	GENERATED_BODY()

public:
	AMCO_Clipboard();

    UFUNCTION(BlueprintImplementableEvent, Category="Clipboard")
    void SetText(const FText& SomeText);
};
