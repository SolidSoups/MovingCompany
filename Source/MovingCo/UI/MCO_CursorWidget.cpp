#include "MCO_CursorWidget.h"
#include "Components/Image.h"
#include "Events/MCO_EventSubsystem.h"
#include "PaperSprite.h"

void UMCO_CursorWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    SetMode(EMCO_CursorMode::None);

    if(UMCO_EventSubsystem* EventSS = UMCO_EventSubsystem::Get(this)){
        EventSS->GetLocalEvents()->OnInteractionTargetChanged.AddUniqueDynamic(this, &UMCO_CursorWidget::HandleInteractionTargetChanged);
    }
}

void UMCO_CursorWidget::NativeDestruct()
{
    if(UMCO_EventSubsystem* EventSS = UMCO_EventSubsystem::Get(this)){
        EventSS->GetLocalEvents()->OnInteractionTargetChanged.RemoveDynamic(this, &UMCO_CursorWidget::HandleInteractionTargetChanged);
    }

    Super::NativeDestruct();
}

void UMCO_CursorWidget::SetMode(EMCO_CursorMode Mode)
{
    switch (Mode)
    {
    case EMCO_CursorMode::None:
        TrySetImage(DefaultCursorTextures.DefaultCursorImage);
        break;
    case EMCO_CursorMode::Interactable:
        TrySetImage(DefaultCursorTextures.InteractableCursorImage);
        break;
    default:
        CursorImage->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UMCO_CursorWidget::HandleInteractionTargetChanged(APawn* Interactor, UMCO_InteractableComponent* NewTarget)
{
    if(Interactor != GetOwningPlayerPawn())
        return;

    SetMode(NewTarget ? EMCO_CursorMode::Interactable : EMCO_CursorMode::None);
}

void UMCO_CursorWidget::TrySetImage(UPaperSprite* Sprite){
    if(IsValid(Sprite)){
        CursorImage->SetVisibility(ESlateVisibility::Visible);
        CursorImage->SetBrushResourceObject(Sprite);
    }
    else{
        CursorImage->SetVisibility(ESlateVisibility::Collapsed);
        CursorImage->SetBrushFromTexture(nullptr);
    }
}
