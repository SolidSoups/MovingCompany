#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "MCO_WidgetStackManager.generated.h"

class UCommonActivatableWidgetStack;
class UCommonActivatableWidget;

USTRUCT()
struct FMCO_WidgetStackInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag LayerTag;

	UPROPERTY()
	TObjectPtr<UCommonActivatableWidgetStack> LayerStack;
};

UCLASS()
class MOVINGCO_API UMCO_WidgetStackManager : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	virtual bool Initialize() override;

	UFUNCTION(BlueprintCallable, meta=(Categories="UI.Layer"))
	UCommonActivatableWidget* PushWidgetToLayerStack(FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, meta=(Categories="UI.Layer"))
	void PopWidgetFromLayerStack(FGameplayTag LayerTag);

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> MenuStack;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> PopupStack;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> GameStack;

	UPROPERTY()
	TArray<FMCO_WidgetStackInfo> WidgetStacks;
};
