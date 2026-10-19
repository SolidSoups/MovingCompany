#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MCO_StressUserWidget.generated.h"

class UProgressBar;
class UTextBlock;

/**
 *  Stress bar. Builds a simple default layout when used directly,
 *  a Blueprint subclass can provide its own (optionally binding StressBar / StressLabel)
 */
UCLASS()
class MOVINGCO_API UMCO_StressUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    void SetStress(float Percent, bool bPanicking);

    UFUNCTION(BlueprintImplementableEvent, Category="MCO|Stress Bar")
    void OnStressChanged(float Percent, bool bPanicking);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

    UPROPERTY(BlueprintReadOnly, Category="MCO|Stress Bar", meta=(BindWidgetOptional))
    TObjectPtr<UProgressBar> StressBar;

    UPROPERTY(BlueprintReadOnly, Category="MCO|Stress Bar", meta=(BindWidgetOptional))
    TObjectPtr<UTextBlock> StressLabel;

private:
    float LastPercent = -1.f;
    bool bLastPanicking = false;
};
