#include "MCO_CursorWidget.h"
#include "Components/Image.h"
#include "PaperSprite.h"

void UMCO_CursorWidget::NativeConstruct()
{
    Super::NativeConstruct();
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
