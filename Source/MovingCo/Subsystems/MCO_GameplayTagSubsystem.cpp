#include "MCO_GameplayTagSubsystem.h"

#include "MovingCo.h"

static void FailPrint(const TCHAR* TagName)
{
	UE_LOG(LogMovingCo, Error, TEXT("Request failed!"));
}

void UMCO_GameplayTagSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// UI Layers
	Tags.UILayerTag = FGameplayTag::RequestGameplayTag(FName("UI.Layer"));
	Tags.UILayerTag_Menu = FGameplayTag::RequestGameplayTag(FName("UI.Layer.Menu"));
	Tags.UILayerTag_Popup = FGameplayTag::RequestGameplayTag(FName("UI.Layer.Popup"));
	Tags.UILayerTag_Game = FGameplayTag::RequestGameplayTag(FName("UI.Layer.Game"));

	// Verification
	if (!Tags.UILayerTag.IsValid())       FailPrint(TEXT("UI.Layer"));
	if (!Tags.UILayerTag_Menu.IsValid())  FailPrint(TEXT("UI.Layer.Menu"));
	if (!Tags.UILayerTag_Popup.IsValid()) FailPrint(TEXT("UI.Layer.Popup"));
	if (!Tags.UILayerTag_Game.IsValid())  FailPrint(TEXT("UI.Layer.Game"));
}
