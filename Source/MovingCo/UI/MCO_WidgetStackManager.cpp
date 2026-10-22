#include "MCO_WidgetStackManager.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "CommonActivatableWidget.h"
#include "MovingCo.h"
#include "Subsystems/MCO_GameplayTagSubsystem.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

bool UMCO_WidgetStackManager::Initialize()
{
	if (!Super::Initialize()) { return false; }

	// Creating an overlay as the root widget, so that layers can have arbitrary layouts
	UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>();
	WidgetTree->RootWidget = Overlay;

	GameStack = WidgetTree->ConstructWidget<UCommonActivatableWidgetStack>();
	MenuStack = WidgetTree->ConstructWidget<UCommonActivatableWidgetStack>();
	PopupStack = WidgetTree->ConstructWidget<UCommonActivatableWidgetStack>();
	WidgetStacks.Add({ UMCO_GameplayTagSubsystem::Tags.UILayerTag_Game, GameStack  });
	WidgetStacks.Add({ UMCO_GameplayTagSubsystem::Tags.UILayerTag_Menu, MenuStack  });
	WidgetStacks.Add({ UMCO_GameplayTagSubsystem::Tags.UILayerTag_Popup, PopupStack });

	// Add stacks to Root Overlay and set them to fill
	for (auto& [Tag, Stack] : WidgetStacks)
	{
		UOverlaySlot* StackSlot = Overlay->AddChildToOverlay(Stack);
		StackSlot->SetHorizontalAlignment(HAlign_Fill);
		StackSlot->SetVerticalAlignment(VAlign_Fill);
	}

	return true;
}

UCommonActivatableWidget* UMCO_WidgetStackManager::PushWidgetToLayerStack(FGameplayTag LayerTag,
	TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (!IsValid(WidgetClass))
	{
		return nullptr;
	}

	FMCO_WidgetStackInfo* Layer = FindLayer(LayerTag);
	if (Layer == nullptr)
	{
		return nullptr;
	}

	return Layer->LayerStack->AddWidget(WidgetClass);
}

void UMCO_WidgetStackManager::PopWidgetFromLayerStack(FGameplayTag LayerTag)
{
	FMCO_WidgetStackInfo* Layer = FindLayer(LayerTag);
	if (Layer == nullptr)
	{
		return;
	}

	UCommonActivatableWidget* ActiveWidget = Layer->LayerStack->GetActiveWidget();
	if (ActiveWidget == nullptr)
	{
		return;
	}

	Layer->LayerStack->RemoveWidget(*ActiveWidget);
}

FMCO_WidgetStackInfo* UMCO_WidgetStackManager::FindLayer(FGameplayTag LayerTag)
{
	FMCO_WidgetStackInfo* Layer = WidgetStacks.FindByPredicate([&LayerTag](const FMCO_WidgetStackInfo& Pair)
	{
		return Pair.LayerTag == LayerTag;
	});

	if (Layer == nullptr)
	{
		UE_LOG(LogMovingCo, Warning, TEXT("No UI layer stack registered for tag '%s'"), *LayerTag.ToString());
	}

	return Layer;
}
