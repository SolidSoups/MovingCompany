#include "MCO_HUDBase.h"

#include "MCO_WidgetStackManager.h"
#include "MovingCo.h"

void AMCO_HUDBase::BeginPlay()
{
	Super::BeginPlay();

	Initialize();
}

void AMCO_HUDBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(WidgetStackManager))
	{
		WidgetStackManager->RemoveFromParent();
		WidgetStackManager = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AMCO_HUDBase::Initialize()
{
	if (IsValid(WidgetStackManager)) { return; }

	APlayerController* OwningPlayerController = GetOwningPlayerController();
	if (!IsValid(OwningPlayerController) || !OwningPlayerController->IsLocalController()) { return; }

	WidgetStackManager = CreateWidget<UMCO_WidgetStackManager>(OwningPlayerController, UMCO_WidgetStackManager::StaticClass());
	if (!IsValid(WidgetStackManager))
	{
		UE_LOG(LogMovingCo, Error, TEXT("Failed to create the widget stack manager!"));
		return;
	}

	WidgetStackManager->AddToViewport();
}

UCommonActivatableWidget* AMCO_HUDBase::PushWidgetToLayerStack(FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	Initialize();
	if (!IsValid(WidgetStackManager)) { return nullptr; }

	return WidgetStackManager->PushWidgetToLayerStack(LayerTag, WidgetClass);
}

void AMCO_HUDBase::PopWidgetFromLayerStack(FGameplayTag LayerTag)
{
	if (!IsValid(WidgetStackManager)) { return; }

	WidgetStackManager->PopWidgetFromLayerStack(LayerTag);
}
