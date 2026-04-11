#include "StateVariableFilter.h"

void StateVariableFilter::prepare (double sampleRate) noexcept
{
    sr                = sampleRate;
    needsCoeffUpdate  = true;
    reset();
}

void StateVariableFilter::reset() noexcept
{
    stateL = {};
    stateR = {};
}

void StateVariableFilter::setType (int t) noexcept
{
    type             = static_cast<Type> (juce::jlimit (0, 3, t));
    needsCoeffUpdate = true;
}

void StateVariableFilter::setCutoff (float hz) noexcept
{
    cutoff           = hz;
    needsCoeffUpdate = true;
}

void StateVariableFilter::setResonance (float res) noexcept
{
    resonance        = juce::jlimit (0.0f, 1.0f, res);
    needsCoeffUpdate = true;
}

void StateVariableFilter::setDrive (float d) noexcept
{
    drive = juce::jlimit (0.0f, 1.0f, d);
}

// ============================================================
// Coefficient calculation — Cytomic TPT SVF
// ============================================================
void StateVariableFilter::updateCoefficients() noexcept
{
    const float maxCutoff = static_cast<float> (sr * 0.499);
    const float fc        = juce::jlimit (20.0f, maxCutoff, cutoff);

    const float g = std::tan (juce::MathConstants<float>::pi * fc
                               / static_cast<float> (sr));

    // k: 0 = maximum resonance/self-oscillation, 2 = no resonance.
    // Map resonance [0,1] → k [2.0, 0.05] (0.05 avoids true self-oscillation).
    k  = 2.0f - 1.95f * resonance;
    a1 = 1.0f / (1.0f + g * (g + k));
    a2 = g * a1;
    a3 = g * a2;

    needsCoeffUpdate = false;
}

// ============================================================
// Per-sample processing
// ============================================================
float StateVariableFilter::processSample (float in, ChanState& st) noexcept
{
    if (needsCoeffUpdate)
        updateCoefficients();

    // Optional pre-filter drive (soft clip)
    if (drive > 0.001f)
    {
        const float gain = 1.0f + drive * 9.0f; // up to 10× gain
        in = std::tanh (in * gain) / std::tanh (gain); // normalised to unity peak
    }

    // --- TPT SVF stage 1 ---
    const float v3_1 = in - st.s2_1;
    const float v1_1 = a1 * st.s1_1 + a2 * v3_1;
    const float v2_1 = st.s2_1 + a2 * st.s1_1 + a3 * v3_1;
    st.s1_1 = 2.0f * v1_1 - st.s1_1;
    st.s2_1 = 2.0f * v2_1 - st.s2_1;

    // Outputs from stage 1
    const float lp1 = v2_1;
    const float bp1 = v1_1;
    const float hp1 = in - k * v1_1 - v2_1;

    switch (type)
    {
        case Type::LP12: return lp1;
        case Type::HP:   return hp1;
        case Type::BP:   return bp1;

        case Type::LP24:
        {
            // --- TPT SVF stage 2 (input = LP output of stage 1) ---
            const float v3_2 = lp1 - st.s2_2;
            const float v1_2 = a1 * st.s1_2 + a2 * v3_2;
            const float v2_2 = st.s2_2 + a2 * st.s1_2 + a3 * v3_2;
            st.s1_2 = 2.0f * v1_2 - st.s1_2;
            st.s2_2 = 2.0f * v2_2 - st.s2_2;
            return v2_2;
        }
    }
    return lp1;
}

float StateVariableFilter::processLeft (float in) noexcept
{
    return processSample (in, stateL);
}

float StateVariableFilter::processRight (float in) noexcept
{
    return processSample (in, stateR);
}
