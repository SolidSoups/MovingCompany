#include "MCO_HUDBase.h"

#include "MCO_WidgetStackManager.h"

void AMCO_HUDBase::Initialize()
{
	if (IsValid(WidgetStackManager)) { return; }

	WidgetStackManager = CreateWidget<UMCO_WidgetStackManager>(
		GetOwningPlayerController(), UMCO_WidgetStackManager::StaticClass());
	WidgetStackManager->AddToViewport();
}

UCommonActivatableWidget* AMCO_HUDBase::PushWidgetToLayerStack(FGameplayTag LayerTag,
                                                              TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	return WidgetStackManager->PushWidgetToLayerStack(LayerTag, WidgetClass);
}
