#include "Oscillator.h"

// ============================================================
// Public interface
// ============================================================

void Oscillator::prepare (double sr) noexcept
{
    sampleRate  = sr;
    needsUpdate = true;
}

void Oscillator::setWaveform (int wf) noexcept
{
    waveform    = static_cast<Waveform> (juce::jlimit (0, 3, wf));
    // No geometry recompute needed — waveform doesn't affect freq/pan.
}

void Oscillator::setFrequency (float hz) noexcept
{
    baseFrequency = hz;
    needsUpdate   = true;
}

void Oscillator::setUnisonVoices (int v) noexcept
{
    voices      = juce::jlimit (1, kMaxUnison, v);
    needsUpdate = true;
}

void Oscillator::setUnisonDetune (float cents) noexcept
{
    unisonDetune = cents;
    needsUpdate  = true;
}

void Oscillator::setUnisonSpread (float s) noexcept
{
    unisonSpread = juce::jlimit (0.0f, 1.0f, s);
    needsUpdate  = true;
}

void Oscillator::setPitchOffsetSemitones (float semi) noexcept
{
    pitchOffsetSemitones = semi;
    needsUpdate          = true;
}

void Oscillator::reset() noexcept
{
    juce::Random rng;
    for (auto& p : phases)
        p = rng.nextDouble();
    needsUpdate = true;
}

// ============================================================
// Per-sample processing
// ============================================================

std::array<float, 2> Oscillator::processSample() noexcept
{
    if (needsUpdate)
        updateVoiceData();

    float sumL = 0.0f, sumR = 0.0f;
    const float norm = 1.0f / static_cast<float> (voices);

    for (int v = 0; v < voices; ++v)
    {
        const auto& vd  = voiceData[v];
        const float ph  = static_cast<float> (phases[v]);
        const float smp = generateSample (waveform, ph, vd.phaseInc);

        sumL += smp * vd.panL;
        sumR += smp * vd.panR;

        phases[v] += vd.phaseInc;
        if (phases[v] >= 1.0) phases[v] -= 1.0;
    }

    return { sumL * norm, sumR * norm };
}

// ============================================================
// Private helpers
// ============================================================

void Oscillator::updateVoiceData() noexcept
{
    const float totalFreq = baseFrequency
                          * std::pow (2.0f, pitchOffsetSemitones / 12.0f);
    const float invSR     = static_cast<float> (1.0 / sampleRate);

    for (int v = 0; v < voices; ++v)
    {
        // Detune — voices spread symmetrically around center.
        // detuneNorm ∈ [-1, +1] for v ∈ [0, voices-1].
        const float detuneNorm = (voices > 1)
            ? (static_cast<float> (v) - (voices - 1) * 0.5f) / ((voices - 1) * 0.5f)
            : 0.0f;

        const float detuneCents = detuneNorm * unisonDetune;
        const float voiceFreq   = totalFreq * std::pow (2.0f, detuneCents / 1200.0f);

        voiceData[v].phaseInc = juce::jlimit (0.0f, 0.499f, voiceFreq * invSR);

        // Constant-power stereo spread.
        const float pan      = (voices > 1) ? (detuneNorm * unisonSpread) : 0.0f; // [-1, +1]
        const float angle    = (pan + 1.0f) * 0.5f * juce::MathConstants<float>::halfPi;
        voiceData[v].panL    = std::cos (angle);
        voiceData[v].panR    = std::sin (angle);
    }

    needsUpdate = false;
}

float Oscillator::generateSample (Waveform wf, float phase, float dt) const noexcept
{
    switch (wf)
    {
        case Waveform::Sine:
            return std::sin (juce::MathConstants<float>::twoPi * phase);

        case Waveform::Triangle:
            // Triangle is already band-limited; no BLEP needed.
            return phase < 0.5f ? (4.0f * phase - 1.0f)
                                : (3.0f - 4.0f * phase);

        case Waveform::Saw:
        {
            float saw = 2.0f * phase - 1.0f;
            saw -= polyBlep (phase, dt);
            return saw;
        }

        case Waveform::Square:
        {
            float sq = (phase < 0.5f) ? 1.0f : -1.0f;
            sq += polyBlep (phase, dt);
            sq -= polyBlep (std::fmod (phase + 0.5f, 1.0f), dt);
            return sq;
        }
    }
    return 0.0f;
}

float Oscillator::polyBlep (float phase, float dt) noexcept
{
    if (dt <= 0.0f) return 0.0f;
    if (phase < dt)
    {
        float t = phase / dt;
        return t + t - t * t - 1.0f;
    }
    if (phase > 1.0f - dt)
    {
        float t = (phase - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}
