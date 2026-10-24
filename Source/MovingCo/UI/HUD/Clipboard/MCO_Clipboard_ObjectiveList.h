#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "MCO_Clipboard_ObjectiveList.generated.h"

class UVerticalBox;

UCLASS()
class MOVINGCO_API UMCO_Clipboard_ObjectiveList : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UVerticalBox> ObjectiveContainer;
};
