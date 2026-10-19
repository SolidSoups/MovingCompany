#pragma once

#include "Containers/Ticker.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MCO_Door.generated.h"

USTRUCT(BlueprintType)
struct FMCO_MoveState
{
    GENERATED_BODY()

    // Is the door opening or closing?
    UPROPERTY(BlueprintReadOnly) bool bOpen = false;
    // The starting time of the open action, server time.
    UPROPERTY(BlueprintReadOnly) double StartTime = -1.f;
};

UCLASS()
class MOVINGCO_API AMCO_Door : public AActor
{
    GENERATED_BODY()

public:
    AMCO_Door(const FObjectInitializer& ObjectInitializer);
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

    static const FName DoorSceneCompName;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    TObjectPtr<class UMCO_InteractableComponent> InteractableComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
    TObjectPtr<class USceneComponent> SceneRootComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
    TObjectPtr<class USceneComponent> DoorSceneComp;



    // Called when we want to close the door
    UFUNCTION(BlueprintImplementableEvent, Category = "Door")
    void OnOpenDoor(FMCO_MoveState InMoveState);
    // Called when we want to open the door
    UFUNCTION(BlueprintImplementableEvent, Category = "Door")
    void OnCloseDoor(FMCO_MoveState InMoveState);

private:
    UPROPERTY(ReplicatedUsing = OnRep_MoveState)
    FMCO_MoveState MoveState;
    UFUNCTION()
    void OnRep_MoveState();

    UFUNCTION()
    void HandleOnInteractedWith(APawn* InstigatorPawn);
};
