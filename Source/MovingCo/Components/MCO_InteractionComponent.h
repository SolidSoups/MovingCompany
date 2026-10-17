#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "MCO_InteractionComponent.generated.h"

// A component which allows the player to interact with interactables.
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOVINGCO_API UMCO_InteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMCO_InteractionComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_DoInteraction();

protected:
    UPROPERTY(EditAnywhere, Category="Debug")
    bool bDebug = true;

    UPROPERTY(EditDefaultsOnly, Category="Interaction")
    float DetectionSphereRadius = 120.f;

    // The minimum alignment needed between the screen cursor
    // and an interactable on screen
    UPROPERTY(EditDefaultsOnly, Category="Interaction",
              meta=(ClampMin="0.5", ClampMax="1.0"))
    float MinimumInteractionAlignement = 0.90f;

private:
    // Finds the best interactable that the player probably
    // wants to interact with
    class UMCO_InteractableComponent* FindBestInteractable(const FVector CameraLocation, const FVector CameraDirection) const;
};
