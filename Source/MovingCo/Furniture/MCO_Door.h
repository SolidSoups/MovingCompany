#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Containers/Ticker.h"
#include "MCO_Door.generated.h"

UCLASS()
class MOVINGCO_API AMCO_Door : public AActor
{
	GENERATED_BODY()

public:
	AMCO_Door();
	virtual void BeginPlay() override;
    virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
    TObjectPtr<class UMCO_InteractableComponent> InteractableComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Door")
    TObjectPtr<class UStaticMeshComponent> StaticMeshComp;

    UPROPERTY(EditDefaultsOnly, Category="Door", meta=(Unit = "°/s"))
    float OpenSpeed = 35.f;

    // The relative yaw of the door. used to turn it open/closed
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_DoorYaw)
    float DoorYaw = 0.0f;

private:
    // Every machine utilizes the OnReplication for DoorYaw to set the 
    // rotation of the door. That way it will always stay synced
    UFUNCTION()
    void OnRep_DoorYaw();

    UFUNCTION()
    void HandleOnInteractedWith(APawn* InstigatorPawn);

    // Tick moving the door to the open or closed position
    bool TickMovingDoor(float DeltaTime, bool bShouldOpen);

    // Is the door in the process of opening/closing?
    bool bMoving;

    // Is the door currently open, or closed?
    bool bOpen = false;

    FTSTicker::FDelegateHandle MovingTickHandle;
};
