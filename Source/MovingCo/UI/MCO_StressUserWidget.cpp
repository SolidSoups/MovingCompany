#include "MCO_StressUserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

TSharedRef<SWidget> UMCO_StressUserWidget::RebuildWidget()
{
    // No designer layout, build a bar at the top of the screen so the prototype works without assets
    if (WidgetTree and !WidgetTree->RootWidget)
    {
        UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
        WidgetTree->RootWidget = Root;

        StressLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StressLabel"));
        StressLabel->SetText(FText::FromString(TEXT("STRESS")));
        StressLabel->SetJustification(ETextJustify::Center);
        StressLabel->SetShadowOffset(FVector2D(1.f, 1.f));
        StressLabel->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.8f));
        FSlateFontInfo Font = StressLabel->GetFont();
        Font.Size = 16;
        StressLabel->SetFont(Font);

        if (UCanvasPanelSlot* LabelSlot = Root->AddChildToCanvas(StressLabel))
        {
            LabelSlot->SetAnchors(FAnchors(0.5f, 0.f));
            LabelSlot->SetAlignment(FVector2D(0.5f, 0.f));
            LabelSlot->SetPosition(FVector2D(0.f, 20.f));
            LabelSlot->SetAutoSize(true);
        }

        StressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("StressBar"));
        StressBar->SetPercent(0.f);

        if (UCanvasPanelSlot* BarSlot = Root->AddChildToCanvas(StressBar))
        {
            BarSlot->SetAnchors(FAnchors(0.5f, 0.f));
            BarSlot->SetAlignment(FVector2D(0.5f, 0.f));
            BarSlot->SetPosition(FVector2D(0.f, 48.f));
            BarSlot->SetSize(FVector2D(360.f, 18.f));
        }
    }

    return Super::RebuildWidget();
}

void UMCO_StressUserWidget::SetStress(float Percent, bool bPanicking)
{
    if (FMath::IsNearlyEqual(Percent, LastPercent, 0.001f) and bPanicking == bLastPanicking)
        return;

    const bool bPanicChanged = bPanicking != bLastPanicking or LastPercent < 0.f;
    LastPercent = Percent;
    bLastPanicking = bPanicking;

    if (StressBar)
    {
        StressBar->SetPercent(Percent);
        StressBar->SetFillColorAndOpacity(bPanicking
            ? FLinearColor(0.6f, 0.1f, 0.9f)
            : FLinearColor::LerpUsingHSV(FLinearColor(1.f, 0.85f, 0.1f), FLinearColor(0.9f, 0.05f, 0.05f), Percent));
    }

    if (StressLabel and bPanicChanged)
    {
        StressLabel->SetText(FText::FromString(bPanicking
            ? TEXT("PANICKING! Calming down...")
            : TEXT("STRESS")));
    }

    OnStressChanged(Percent, bPanicking);
}
