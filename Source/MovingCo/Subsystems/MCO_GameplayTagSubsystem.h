#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MCO_GameplayTagSubsystem.generated.h"

struct FMCO_GameplayTagSubsystemTags
{
	// UI Layers
	FGameplayTag UILayerTag;
	FGameplayTag UILayerTag_Menu;
	FGameplayTag UILayerTag_Popup;
	FGameplayTag UILayerTag_Game;
};


UCLASS()
class MOVINGCO_API UMCO_GameplayTagSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	inline static FMCO_GameplayTagSubsystemTags Tags;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};
