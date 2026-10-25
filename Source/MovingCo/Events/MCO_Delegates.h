#pragma once

#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "MCO_Delegates.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FMCO_OnValuableDamagedNativeSignature, float /* ValueDamage */, const FVector& /* Location */);
UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMCO_OnValuableDamagedDynamicSignature, float, ValueDamage, const FVector&, Location);
