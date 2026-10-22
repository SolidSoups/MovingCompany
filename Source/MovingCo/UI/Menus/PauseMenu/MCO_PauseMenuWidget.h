#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "MCO_PauseMenuWidget.generated.h"

class UButton;

UCLASS()
class MOVINGCO_API UMCO_PauseMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> ResumeButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> RestartButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> MainMenuButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> QuitGameButton;
};
