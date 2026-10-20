#pragma once

#include "Containers/Ticker.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MCO_Door.generated.h"

class UMCO_ObjectiveUserWidget;

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
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

    /** Removes the highlight and the objective text for good, e.g. once the objective is complete */
    UFUNCTION(BlueprintCallable, Category="Door|Objective")
    void ClearObjective();

    static const FName DoorSceneCompName;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    TObjectPtr<class UMCO_InteractableComponent> InteractableComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
    TObjectPtr<class USceneComponent> SceneRootComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
    TObjectPtr<class USceneComponent> DoorSceneComp;

    /** Marks this door as the objective: it shows through walls until the local player first gets close, with a hint on screen */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Door|Objective")
    bool bIsObjective = false;

    /** Overlay drawn on the door's meshes. Use a translucent unlit material with Disable Depth Test ticked so it shows through walls */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Door|Objective", meta=(EditCondition="bIsObjective"))
    TObjectPtr<class UMaterialInterface> HighlightMaterial;

    /** The highlight goes away for good once the local player is this close to the door */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Door|Objective", meta=(EditCondition="bIsObjective", ClampMin="0", Units="cm"))
    float HighlightRevealDistance = 500.f;

    /** Explains how to complete the objective. Leave empty for no text overlay */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Door|Objective", meta=(EditCondition="bIsObjective", MultiLine="true"))
    FText ObjectiveText;

    UPROPERTY(EditAnywhere, Category="Door|Objective", meta=(EditCondition="bIsObjective"))
    TSubclassOf<UMCO_ObjectiveUserWidget> ObjectiveWidgetClass;

    // Called the first time the local player gets within HighlightRevealDistance
    UFUNCTION(BlueprintImplementableEvent, Category="Door|Objective")
    void OnObjectiveReached();

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

    void UpdateObjective();
    void SetHighlighted(bool bHighlighted);

    UPROPERTY()
    TObjectPtr<UMCO_ObjectiveUserWidget> ObjectiveWidget;

    // Local only, every client reaches the door on its own
    bool bObjectiveReached = false;
    bool bObjectiveCleared = false;
};
