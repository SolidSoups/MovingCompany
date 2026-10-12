#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/HUD.h"
#include "MCO_HUDBase.generated.h"

class UMCO_WidgetStackManager;
class UCommonActivatableWidget;

UCLASS()
class MOVINGCO_API AMCO_HUDBase : public AHUD
{
	GENERATED_BODY()
	
public:
	void Initialize();

	UFUNCTION(BlueprintCallable, Category="HUD", meta=(Categories="UI.Layer"))
	UCommonActivatableWidget* PushWidgetToLayerStack(FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass);

	template<class TWidgetClass>
	requires TIsDerivedFrom<TWidgetClass, UCommonActivatableWidget>::IsDerived
	UCommonActivatableWidget* PushWidgetToLayerStack(FGameplayTag LayerTag)
	{
		return Cast<TWidgetClass>(PushWidgetToLayerStack(LayerTag, TWidgetClass::StaticClass()));
	}

private:
	UPROPERTY()
	TObjectPtr<UMCO_WidgetStackManager> WidgetStackManager;
};
