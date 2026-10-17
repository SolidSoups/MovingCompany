#pragma once

#include "Containers/Ticker.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MCO_Door.generated.h"

USTRUCT()
struct FMCO_MoveState
{
    GENERATED_BODY()

    UPROPERTY() bool bOpen = false;
    UPROPERTY() double StartTime = -1.f;
};

UCLASS()
class MOVINGCO_API AMCO_Door : public AActor
{
    GENERATED_BODY()

public:
    AMCO_Door();
    virtual void BeginPlay() override;
    virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    TObjectPtr<class UMCO_InteractableComponent> InteractableComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Door")
    TObjectPtr<class UStaticMeshComponent> StaticMeshComp;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Door",
              meta = (Units = "s", ClampMin="0.01"))
    float MoveDuration = 2.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Door")
    TObjectPtr<class UCurveFloat> OpenCurve;

    UPROPERTY(EditDefaultsOnly, Category = "Door")
    TObjectPtr<class UCurveFloat> CloseCurve;

    UPROPERTY(EditDefaultsOnly, Category = "Door", meta = (Unit = "°/s"))
    float OpenSpeed = 35.f;

    UPROPERTY(EditDefaultsOnly, Category = "Door", meta = (Unit = "°", ClampMin = "0.0", ClampMax = "180.0"));
    float OpeningAngle = -90.f;


    UPROPERTY(EditAnywhere, Category = "Door");
    bool bDebug = true;

    UPROPERTY(EditDefaultsOnly, Category = "Door", meta = (ClampMin = "0.0"));
    float SmoothingDecayRate = 1.f;

    // The relative yaw of the door. used to turn it open/closed
    UPROPERTY(BlueprintReadOnly, Replicated)
    float DoorYawTarget = 0.0f;

    UFUNCTION(BlueprintImplementableEvent, Category = "Door")
    void OnDoorClosed();
    UFUNCTION(BlueprintImplementableEvent, Category = "Door")
    void OnDoorOpened();

private:
    UPROPERTY(Replicated)
    FMCO_MoveState MoveState;

    UPROPERTY(Transient)
    float CurrentDoorYaw = 0.0f;

    // The static mesh starts at odd angles that we need to account for its starting rotation
    float StartingYaw = 0.0f;

    // Used to detect when a door has finished closing/opening
    double LastSeenStartTime = -1.0f;
    bool bFXTriggered = true;

    UFUNCTION()
    void HandleOnInteractedWith(APawn* InstigatorPawn);

    // Is the door in the process of opening/closing?
    bool bMoving;

    // Is the door currently open, or closed?
    bool bOpen = false;

    FTSTicker::FDelegateHandle MovingTickHandle;
};
