// Copyright Epic Games, Inc. All Rights Reserved.

#include "MovingCoPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Destruction/MCO_DestructibleActor.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Input/CommonUIActionRouterBase.h"
#include "InputMappingContext.h"
#include "MCO_MainMenuGameMode.h"
#include "MovingCo.h"
#include "MovingCoCameraManager.h"
#include "Networking/MCO_SessionSubsystem.h"
#include "Subsystems/MCO_GameplayTagSubsystem.h"
#include "UI/MCO_HUDBase.h"
#include "UI/HUD/MCO_PlayerHUDBase.h"
#include "UI/Menus/MainMenu/MCO_MainMenuWidget.h"
#include "Widgets/Input/SVirtualJoystick.h"

AMovingCoPlayerController::AMovingCoPlayerController()
{
    // set the player camera manager class
    PlayerCameraManagerClass = AMovingCoCameraManager::StaticClass();
}

void AMovingCoPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocalController())
    {
        auto World = GetWorld();
        if (!IsValid(World))
            return;

        auto* GameModeBase = World->GetAuthGameMode();
        if (IsValid(GameModeBase) && GameModeBase->IsA<AMCO_MainMenuGameMode>())
        {
            if (!IsValid(MainMenuWidgetClass))
            {
                UE_LOG(LogMovingCo, Error, TEXT("MainMenuWidgetClass is not set on %s!"), *GetName());
            }

            // The HUD creates its own widget stack manager; pushing initialises it if it hasn't yet
            if (AMCO_HUDBase* HUD = GetHUDBase())
            {
                HUD->PushWidgetToLayerStack(UMCO_GameplayTagSubsystem::Tags.UILayerTag_Menu, MainMenuWidgetClass);
                SetMCOInputMode(EMCO_InputMode::UI);
                return;
            }
        }
        
        if (!IsValid(PlayerHUDClass))
        {
            UE_LOG(LogMovingCo, Error, TEXT("PlayerHUDBaseClass is not set on %s!"), *GetName());
        }
            
        if (AMCO_HUDBase* HUD = GetHUDBase())
        {
            HUD->PushWidgetToLayerStack(UMCO_GameplayTagSubsystem::Tags.UILayerTag_Game, PlayerHUDClass);
            SetMCOInputMode(EMCO_InputMode::Game);
            return;
        }

        // only spawn touch controls on local player controllers
        // TODO [Elias]: Don't think we need touch controls, we should strip it out.
        if (ShouldUseTouchControls())
        {
            // spawn the mobile controls widget
            MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

            if (MobileControlsWidget)
            {
                // add the controls to the player screen
                MobileControlsWidget->AddToPlayerScreen(0);
            }
            else
            {
                UE_LOG(LogMovingCo, Error, TEXT("Could not spawn mobile controls widget."));
            }
        }
    }
}

void AMovingCoPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // only add IMCs for local player controllers
    if (IsLocalPlayerController())
    {
        // Add Input Mapping Context
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
                ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
            {
                Subsystem->AddMappingContext(CurrentContext, 0);
            }

            // only add these IMCs if we're not using mobile touch input
            if (!ShouldUseTouchControls())
            {
                for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
                {
                    Subsystem->AddMappingContext(CurrentContext, 0);
                }
            }

            if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
            {
                // Used for the Destructible wall test
                // EnhancedInputComponent->BindAction(PushAction, ETriggerEvent::Started, this,
                // &AMovingCoPlayerController::DoPush); EnhancedInputComponent->BindAction(ResetAction,
                // ETriggerEvent::Started, this, &AMovingCoPlayerController::DoReset);
            }
        }
    }
}

bool AMovingCoPlayerController::ShouldUseTouchControls() const
{
    // are we on a mobile platform? Should we force touch?
    return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

AMCO_HUDBase* AMovingCoPlayerController::GetHUDBase()
{
    // MyHUD can arrive after BeginPlay on network clients (ClientSetHUD), so resolve lazily
    if (!IsValid(HUDBase))
    {
        HUDBase = Cast<AMCO_HUDBase>(GetHUD());
    }

    return HUDBase;
}

void AMovingCoPlayerController::SetMCOInputMode(EMCO_InputMode Mode)
{
    SetShowMouseCursor(Mode != EMCO_InputMode::Game);

    // CommonUI's action router owns input routing; it ignores SetInputMode and keeps its own mode across map travel
    UCommonUIActionRouterBase* ActionRouter = ULocalPlayer::GetSubsystem<UCommonUIActionRouterBase>(GetLocalPlayer());
    if (!IsValid(ActionRouter))
    {
        UE_LOG(LogMovingCo, Error, TEXT("No CommonUI action router for %s, falling back to SetInputMode"), *GetName());

        switch (Mode)
        {
        case EMCO_InputMode::Game:      SetInputMode(FInputModeGameOnly());  break;
        case EMCO_InputMode::UI:        SetInputMode(FInputModeUIOnly());    break;
        case EMCO_InputMode::GameAndUI: SetInputMode(FInputModeGameAndUI()); break;
        }
        return;
    }

    switch (Mode)
    {
    case EMCO_InputMode::Game:
        ActionRouter->SetActiveUIInputConfig(FUIInputConfig(ECommonInputMode::Game, EMouseCaptureMode::CapturePermanently), this);
        break;
    case EMCO_InputMode::UI:
        ActionRouter->SetActiveUIInputConfig(FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture), this);
        break;
    case EMCO_InputMode::GameAndUI:
        ActionRouter->SetActiveUIInputConfig(FUIInputConfig(ECommonInputMode::All, EMouseCaptureMode::CaptureDuringMouseDown), this);
        break;
    }
}

void AMovingCoPlayerController::HostGame()
{
    GetGameInstance()->GetSubsystem<UMCO_SessionSubsystem>()->HostSession();
}

void AMovingCoPlayerController::JoinGame()
{
    GetGameInstance()->GetSubsystem<UMCO_SessionSubsystem>()->FindAndJoinSession();
}
