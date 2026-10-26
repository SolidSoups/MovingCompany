#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MCO_Events.h"
#include "MCO_EventSubsystem.generated.h"

UCLASS()
class MOVINGCO_API UMCO_EventSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
    static UMCO_EventSubsystem* Get(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintPure, Category="Events")
    UMCO_AllEvents* GetAllEvents() const { return AllEvents; }

    // Returns null on clients, as server events can only be used on the server.
    UFUNCTION(BlueprintPure, Category="Events")
    UMCO_ServerEvents* GetServerEvents() const;

    UFUNCTION(BlueprintPure, Category="Events")
    UMCO_LocalEvents* GetLocalEvents() const { return LocalEvents; }

protected:
    virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
    UPROPERTY()
    TObjectPtr<UMCO_AllEvents> AllEvents;
    UPROPERTY()
    TObjectPtr<UMCO_ServerEvents> ServerEvents;
    UPROPERTY()
    TObjectPtr<UMCO_LocalEvents> LocalEvents;
};
