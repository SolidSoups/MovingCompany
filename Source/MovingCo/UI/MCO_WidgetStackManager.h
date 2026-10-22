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
	FMCO_WidgetStackInfo* FindLayer(FGameplayTag LayerTag);

	UPROPERTY()
	TObjectPtr<UCommonActivatableWidgetStack> GameStack;

	UPROPERTY()
	TObjectPtr<UCommonActivatableWidgetStack> MenuStack;

	UPROPERTY()
	TObjectPtr<UCommonActivatableWidgetStack> PopupStack;

	UPROPERTY()
	TArray<FMCO_WidgetStackInfo> WidgetStacks;
};
