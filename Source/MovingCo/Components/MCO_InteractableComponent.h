#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "MCO_InteractableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractedWith, APawn*, Instigator);

// A component which can be interacted with by the player
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), DisplayName="Interactable Scene Component")
class MOVINGCO_API UMCO_InteractableComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UMCO_InteractableComponent();
    virtual void BeginPlay() override;


    UFUNCTION(BlueprintCallable, Category="Interaction")
    void InteractWith(APawn* Instigator);

    UPROPERTY(BlueprintAssignable, Category="Interaction")
    FOnInteractedWith OnInteractedWith;
protected:
    // The player will use this InteractionComponent will this collider
    // to find the InteractableComponent
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Collision")
    TObjectPtr<class USphereComponent> SphereComp;
};
