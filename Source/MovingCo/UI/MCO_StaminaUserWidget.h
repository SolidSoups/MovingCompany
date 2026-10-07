#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MCO_StaminaUserWidget.generated.h"

UCLASS()
class MOVINGCO_API UMCO_StaminaUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintImplementableEvent, Category="MCO|Stamina Bar")
    void SetStaminaFill(float Value);

protected:
	virtual void NativeConstruct() override;
};
