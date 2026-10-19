#include "MCO_ProceduralThud.h"
#include "Sound/SoundWaveProcedural.h"
#include "UObject/Package.h"

USoundWaveProcedural* MCO::Audio::CreateThud(float Intensity)
{
    constexpr int32 SampleRate = 44100;
    constexpr int32 NumSamples = static_cast<int32>(SampleRate * ThudDuration);
    constexpr int32 FadeOutSamples = SampleRate / 100;

    Intensity = FMath::Clamp(Intensity, 0.f, 1.f);

    const float ThumpFreq = FMath::FRandRange(60.f, 95.f);
    const float KnockFreq = FMath::FRandRange(170.f, 260.f);
    const float Gain = FMath::Lerp(0.35f, 0.9f, Intensity);

    TArray<int16> Samples;
    Samples.SetNumUninitialized(NumSamples);

    float ThumpPhase = 0.f;
    float KnockPhase = 0.f;
    float FilteredNoise = 0.f;

    for (int32 i = 0; i < NumSamples; ++i)
    {
        const float Time = static_cast<float>(i) / SampleRate;

        // Low body thump with a quick downward pitch sweep
        const float Sweep = ThumpFreq * (1.f + 1.5f * FMath::Exp(-Time * 40.f));
        ThumpPhase += UE_TWO_PI * Sweep / SampleRate;
        const float Thump = FMath::Sin(ThumpPhase) * FMath::Exp(-Time * 14.f);

        // Woody mid knock that dies out faster
        KnockPhase += UE_TWO_PI * KnockFreq / SampleRate;
        const float Knock = 0.5f * FMath::Sin(KnockPhase) * FMath::Exp(-Time * 35.f);

        // Lowpassed noise click for the initial impact
        FilteredNoise += (FMath::FRandRange(-1.f, 1.f) - FilteredNoise) * 0.25f;
        const float Click = 0.8f * Intensity * FilteredNoise * FMath::Exp(-Time * 90.f);

        float Sample = FMath::Tanh((Thump + Knock + Click) * Gain * 1.5f);
        if (i > NumSamples - FadeOutSamples)
            Sample *= static_cast<float>(NumSamples - i) / FadeOutSamples;

        Samples[i] = static_cast<int16>(Sample * 32767.f);
    }

    USoundWaveProcedural* Wave = NewObject<USoundWaveProcedural>(GetTransientPackage());
    Wave->SetSampleRate(SampleRate);
    Wave->NumChannels = 1;
    Wave->Duration = ThudDuration;
    Wave->SoundGroup = SOUNDGROUP_Default;
    Wave->bLooping = false;
    Wave->QueueAudio(reinterpret_cast<const uint8*>(Samples.GetData()), Samples.Num() * sizeof(int16));

    return Wave;
}
