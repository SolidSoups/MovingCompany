#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "MCO_VisionConeComponent.generated.h"

class UMaterialInstanceDynamic;

UENUM(BlueprintType)
enum class EMCO_AlertLevel : uint8
{
    Calm,
    Suspicious,
    Alerted
};

/**
 *  What a watcher can see. Draws itself as a fan on the floor that gets cut off by walls,
 *  and the same rules are used for the actual sight checks so the visual never lies.
 *  Attach it to the capsule and offset it down to the floor.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MOVINGCO_API UMCO_VisionConeComponent : public UProceduralMeshComponent
{
	GENERATED_BODY()

public:
	UMCO_VisionConeComponent(const FObjectInitializer& ObjectInitializer);
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** True if the point is inside the cone and nothing (except IgnoredActors) blocks the view */
    bool CanSeePoint(const FVector& Point, const TArray<const AActor*>& IgnoredActors = {}) const;

    UFUNCTION(BlueprintPure, Category="MCO|Vision")
    bool CanSeeActor(const AActor* Target) const;

    /** Server: turns the cone on/off for everyone */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="MCO|Vision")
    void SetConeActive(bool bInActive);

    UFUNCTION(BlueprintPure, Category="MCO|Vision")
    bool IsConeActive() const { return bConeActive; }

    /** Server: tints the cone for everyone */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="MCO|Vision")
    void SetAlertLevel(EMCO_AlertLevel InLevel);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MCO|Vision", meta=(ClampMin="0"))
    float ViewDistance = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MCO|Vision", meta=(ClampMin="1", ClampMax="179"))
    float ViewHalfAngle = 40.f;

    UPROPERTY(EditAnywhere, Category="MCO|Vision")
    bool bStartActive = true;

    UPROPERTY(EditAnywhere, Category="MCO|Vision|Visual")
    FLinearColor ConeColor = FLinearColor(1.f, 0.f, 0.f, 1.f);

protected:
	virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Category="MCO|Vision|Visual", meta=(ClampMin="2", ClampMax="128"))
    int32 Segments = 40;

    /** Uses a "Color" vector parameter (alpha drives opacity). Falls back to a simple engine translucent material */
    UPROPERTY(EditAnywhere, Category="MCO|Vision|Visual")
    TObjectPtr<UMaterialInterface> ConeMaterial;

    /** Opacity per alert level: Calm, Suspicious, Alerted */
    UPROPERTY(EditAnywhere, Category="MCO|Vision|Visual")
    FVector AlertOpacity = FVector(0.25f, 0.4f, 0.6f);

private:
    FVector GetEyeLocation() const;
    void RebuildMesh();
    void UpdateVisuals();

    UFUNCTION()
    void OnRep_ConeState() { UpdateVisuals(); }

    UPROPERTY(ReplicatedUsing=OnRep_ConeState)
    bool bConeActive = true;

    UPROPERTY(ReplicatedUsing=OnRep_ConeState)
    EMCO_AlertLevel AlertLevel = EMCO_AlertLevel::Calm;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> ConeMID;

    bool bSectionCreated = false;
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
};
