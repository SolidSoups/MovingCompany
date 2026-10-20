#include "MCO_ObjectiveUserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

TSharedRef<SWidget> UMCO_ObjectiveUserWidget::RebuildWidget()
{
    // No designer layout, build a panel in the top left so the prototype works without assets
    if (WidgetTree and !WidgetTree->RootWidget)
    {
        UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
        WidgetTree->RootWidget = Root;

        UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
        Background->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.5f));
        Background->SetPadding(FMargin(16.f, 12.f));

        if (UCanvasPanelSlot* BackgroundSlot = Root->AddChildToCanvas(Background))
        {
            BackgroundSlot->SetAnchors(FAnchors(0.f, 0.f));
            BackgroundSlot->SetPosition(FVector2D(32.f, 32.f));
            BackgroundSlot->SetAutoSize(true);
        }

        UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Box"));
        Background->SetContent(Box);

        UTextBlock* Header = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ObjectiveHeader"));
        Header->SetText(FText::FromString(TEXT("OBJECTIVE")));
        Header->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.85f, 0.1f)));
        FSlateFontInfo HeaderFont = Header->GetFont();
        HeaderFont.Size = 14;
        Header->SetFont(HeaderFont);

        if (UVerticalBoxSlot* HeaderSlot = Box->AddChildToVerticalBox(Header))
            HeaderSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));

        ObjectiveLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ObjectiveLabel"));
        ObjectiveLabel->SetWrapTextAt(420.f);
        ObjectiveLabel->SetShadowOffset(FVector2D(1.f, 1.f));
        ObjectiveLabel->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.8f));
        FSlateFontInfo Font = ObjectiveLabel->GetFont();
        Font.Size = 18;
        ObjectiveLabel->SetFont(Font);

        Box->AddChildToVerticalBox(ObjectiveLabel);
    }

    return Super::RebuildWidget();
}

void UMCO_ObjectiveUserWidget::SetObjectiveText(const FText& InText)
{
    if (ObjectiveLabel)
        ObjectiveLabel->SetText(InText);

    OnObjectiveTextChanged(InText);
}
