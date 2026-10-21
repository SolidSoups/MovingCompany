#include "MCO_Clipboard.h"

AMCO_Clipboard::AMCO_Clipboard()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMCO_Clipboard::BeginPlay()
{
	Super::BeginPlay();
}

void AMCO_Clipboard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
