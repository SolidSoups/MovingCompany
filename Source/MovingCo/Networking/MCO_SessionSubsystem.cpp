#include "Networking/MCO_SessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "MovingCo.h"

// Every developer testing Steam shares app 480.
// We need a key to tag our lobby and find it
static const FName MovingCoLobbyKey(TEXT("MOVINGCO"));

IOnlineSessionPtr UMCO_SessionSubsystem::GetSessionInterface() const
{
    IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetGameInstance()->GetWorld());
    if(!Subsystem)
        return nullptr;
    return Subsystem->GetSessionInterface();
}

void UMCO_SessionSubsystem::HostSession()
{
    IOnlineSessionPtr Session = GetSessionInterface();
    if(!Session.IsValid()){
        UE_LOG(LogMovingCo, Warning, TEXT("HostSession: no session interface"));
        return;
    }

    // One session per name. A leftover from the last test makes CreateSession fail.
    if(Session->GetNamedSession(NAME_GameSession)){
        Session->DestroySession(NAME_GameSession);
        UE_LOG(LogMovingCo, Warning, TEXT("HostSession: destroyed old session, run HostGame again"));
        return;
    }

    FOnlineSessionSettings Settings;
    Settings.NumPublicConnections = 5;
    Settings.bShouldAdvertise = true;
    // Steam recomputes lobby joinability whenever a member joins, using this flag.
    // Without it, the lobby turns unjoinable after the first client.
    Settings.bAllowJoinInProgress = true;
    Settings.bUsesPresence = true;
    Settings.bUseLobbiesIfAvailable = true;
    Settings.Set(MovingCoLobbyKey, FString(TEXT("1")), EOnlineDataAdvertisementType::ViaOnlineService);

    CreateSessionHandle = Session->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));

    if(!Session->CreateSession(0, NAME_GameSession, Settings)){
        Session->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionHandle);
        UE_LOG(LogMovingCo, Warning, TEXT("HostSession: CreateSession failed to start"));
    }
}

void UMCO_SessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful){
    if(IOnlineSessionPtr Session = GetSessionInterface()){
        Session->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionHandle);
    }

    if(!bWasSuccessful){
        UE_LOG(LogMovingCo, Warning, TEXT("HostSession: CreateSession failed"));
        return;
    }

    UE_LOG(LogMovingCo, Log, TEXT("HostSession: lobby created, opening map as listen server"));
    UGameplayStatics::OpenLevel(GetGameInstance(), FName(TEXT("/Game/Developers/elima/Prototypes/LVL_Prototype_CrouchBox")), true, TEXT("listen"));
}

void UMCO_SessionSubsystem::FindAndJoinSession(){
    IOnlineSessionPtr Session = GetSessionInterface();
    if(!Session.IsValid()){
        UE_LOG(LogMovingCo, Warning, TEXT("FindAndJoinSession: no session interface"));
        return;
    }

    // JoinSession fails if this machine still holds a session from hosting earlier.
    if(Session->GetNamedSession(NAME_GameSession)){
        Session->DestroySession(NAME_GameSession);
        UE_LOG(LogMovingCo, Warning, TEXT("FindAndJoinSession: destroyed old session, run JoinGame again"));
        return;
    }

    SessionSearch = MakeShared<FOnlineSessionSearch>();
    SessionSearch->MaxSearchResults = 20;
    SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
    SessionSearch->QuerySettings.Set(MovingCoLobbyKey, FString(TEXT("1")), EOnlineComparisonOp::Equals);

    FindSessionHandle = Session->AddOnFindSessionsCompleteDelegate_Handle(
        FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete));

    if(!Session->FindSessions(0, SessionSearch.ToSharedRef())){
        Session->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionHandle);
        UE_LOG(LogMovingCo, Warning, TEXT("FindAndJoinSession: FindSessions failed to start"));
    }
}

void UMCO_SessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful){
    IOnlineSessionPtr Session = GetSessionInterface();
    if(!Session.IsValid()){
        return;
    }
    Session->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionHandle);

    if(!bWasSuccessful || SessionSearch->SearchResults.IsEmpty()){
        UE_LOG(LogMovingCo, Warning, TEXT("FindAndJoinSession: no lobbies found"));
        return;
    }

    const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[0];
    UE_LOG(LogMovingCo, Log, TEXT("FindAndJoinSession: joining %s's lobby"), *Result.Session.OwningUserName);

    JoinSessionHandle = Session->AddOnJoinSessionCompleteDelegate_Handle(
        FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete));

    if(!Session->JoinSession(0, NAME_GameSession, Result)){
        Session->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionHandle);
        UE_LOG(LogMovingCo, Warning, TEXT("FindAndJoinSession: JoinSession failed to start"));
    }
}

void UMCO_SessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result){
    IOnlineSessionPtr Session = GetSessionInterface();
    if(!Session.IsValid()){
        return;
    }
    Session->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionHandle);

    if(Result != EOnJoinSessionCompleteResult::Success){
        UE_LOG(LogMovingCo, Warning, TEXT("FindAndJoinSession: JoinSession failed: %s"), LexToString(Result));
        return;
    }

    FString Address;
    if(!Session->GetResolvedConnectString(SessionName, Address)){
        UE_LOG(LogMovingCo, Warning, TEXT("FindAndJoinSession: no connect string"));
        return;
    }

    UE_LOG(LogMovingCo, Log, TEXT("FindAndJoinSession: travelling to %s"), *Address);
    if(APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController()){
        PC->ClientTravel(Address, TRAVEL_Absolute);
    }
}
