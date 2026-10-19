#pragma once

#include "CoreMinimal.h"

class USoundWaveProcedural;

namespace MCO::Audio
{
    constexpr float ThudDuration = 0.35f;

    /**
     *  Synthesizes a short furniture "thud" so bumps make noise before real sound assets exist.
     *  Every call is slightly different. Intensity is 0-1.
     *  Procedural waves don't stop on their own, stop the audio component after ThudDuration.
     */
    USoundWaveProcedural* CreateThud(float Intensity);
}
