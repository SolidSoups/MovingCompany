#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "MCO_ClipboardWidget.generated.h"

class UButton;
class UWidgetSwitcher;

UCLASS()
class MOVINGCO_API UMCO_ClipboardWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> TasksTabButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;
};
