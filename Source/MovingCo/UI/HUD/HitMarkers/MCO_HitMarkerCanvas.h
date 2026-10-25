#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MCO_HitMarkerCanvas.generated.h"

UCLASS()
class MOVINGCO_API UMCO_HitMarkerCanvas : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

    UPROPERTY(VisibleAnywhere, Category="Hit Markers", meta=(BindWidget))
    TObjectPtr<class UCanvasPanel> RootCanvasPanel;

};
