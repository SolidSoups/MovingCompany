#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MCO_HitMarkerWidget.generated.h"

UCLASS()
class MOVINGCO_API UMCO_HitMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
};
