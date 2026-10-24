#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "MCO_Clipboard_ObjectiveEntry.generated.h"

class UTextBlock;

UCLASS()
class MOVINGCO_API UMCO_Clipboard_ObjectiveEntry : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemText;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> LocationText;
};
