#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MCO_ObjectiveUserWidget.generated.h"

class UTextBlock;

/**
 *  On screen hint explaining the current objective. Builds a simple default layout when used directly,
 *  a Blueprint subclass can provide its own (optionally binding ObjectiveLabel)
 */
UCLASS()
class MOVINGCO_API UMCO_ObjectiveUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    void SetObjectiveText(const FText& InText);

    UFUNCTION(BlueprintImplementableEvent, Category="MCO|Objective")
    void OnObjectiveTextChanged(const FText& InText);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

    UPROPERTY(BlueprintReadOnly, Category="MCO|Objective", meta=(BindWidgetOptional))
    TObjectPtr<UTextBlock> ObjectiveLabel;
};
