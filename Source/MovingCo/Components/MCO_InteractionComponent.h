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

    UPROPERTY()
    TObjectPtr<class USphereComponent> DetectionSphere;

    UFUNCTION()
    void OnBeginOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor,
                        class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                        const struct FHitResult& SweepResult);
    UFUNCTION()
    void OnEndOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor,
                      class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    // TODO: Should probably be a TWeakObjectPtr
    TArray<class UMCO_InteractableComponent*> Interactables;
};
