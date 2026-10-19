#include "MCO_StressComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "UI/MCO_StressUserWidget.h"

UMCO_StressComponent::UMCO_StressComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);

    StressWidgetClass = UMCO_StressUserWidget::StaticClass();
}

void UMCO_StressComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMCO_StressComponent, Stress);
    DOREPLIFETIME(UMCO_StressComponent, bPanicking);
}

void UMCO_StressComponent::EndPlay(EEndPlayReason::Type Reason)
{
    if (StressWidget)
    {
        StressWidget->RemoveFromParent();
        StressWidget = nullptr;
    }
    Super::EndPlay(Reason);
}

void UMCO_StressComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (GetOwner()->HasAuthority())
    {
        if (bPanicking)
        {
            Stress = FMath::Max(Stress - PanicCalmPerSecond * DeltaTime, 0.f);
            if (Stress <= 0.f)
                SetPanicking(false);
        }
        else
        {
            TimeSinceStressed += DeltaTime;
            if (TimeSinceStressed >= CalmDelay and Stress > 0.f)
                Stress = FMath::Max(Stress - CalmPerSecond * DeltaTime, 0.f);
        }
    }

    UpdateWidget();
}

void UMCO_StressComponent::AddStress(float Amount)
{
    if (!GetOwner()->HasAuthority() or bPanicking or Amount <= 0.f)
        return;

    Stress = FMath::Min(Stress + Amount, MaxStress);
    TimeSinceStressed = 0.f;

    if (Stress >= MaxStress)
        SetPanicking(true);
}

void UMCO_StressComponent::SetPanicking(bool bInPanicking)
{
    if (bPanicking == bInPanicking)
        return;

    bPanicking = bInPanicking;
    OnRep_Panicking();
}

void UMCO_StressComponent::OnRep_Panicking()
{
    OnPanicChanged.Broadcast(bPanicking);
}

void UMCO_StressComponent::UpdateWidget()
{
    // The widget is created lazily since the pawn might not be possessed yet in BeginPlay.
    // It stays up while panicking even though the AI is the one controlling us then
    if (!StressWidget and StressWidgetClass)
    {
        const APawn* Pawn = Cast<APawn>(GetOwner());
        APlayerController* PC = Pawn ? Pawn->GetController<APlayerController>() : nullptr;
        if (!PC or !PC->IsLocalPlayerController())
            return;

        StressWidget = CreateWidget<UMCO_StressUserWidget>(PC, StressWidgetClass);
        if (!StressWidget)
            return;

        StressWidget->AddToViewport();
    }

    if (StressWidget)
        StressWidget->SetStress(GetStressPercent(), bPanicking);
}
