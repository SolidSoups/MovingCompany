#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MCO_SessionSubsystem.generated.h"

class FOnlineSessionSearch;

UCLASS()
class MOVINGCO_API UMCO_SessionSubsystem : public UGameInstanceSubsystem {
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Sessions")
    void HostSession();
    UFUNCTION(BlueprintCallable, Category="Sessions")
    void FindAndJoinSession();

private:
    IOnlineSessionPtr GetSessionInterface() const;

    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void OnFindSessionsComplete(bool bWasSuccessful);
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

    TSharedPtr<FOnlineSessionSearch> SessionSearch;

    FDelegateHandle CreateSessionHandle;
    FDelegateHandle FindSessionHandle;
    FDelegateHandle JoinSessionHandle;
};
