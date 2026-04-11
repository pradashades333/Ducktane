#pragma once
#include <JuceHeader.h>

/**
 * Topology-Preserving Transform (TPT) State Variable Filter.
 *
 * Modes:
 *   LP12 — 12 dB/oct low-pass
 *   LP24 — 24 dB/oct low-pass (two cascaded TPT stages)
 *   HP   — 12 dB/oct high-pass
 *   BP   — band-pass
 *
 * Drive applies a tanh soft-clipper before the first stage,
 * giving the characteristic "driven filter" sound.
 *
 * Stereo: separate state for left and right channels.
 */
class StateVariableFilter
{
public:
    enum class Type { LP12 = 0, LP24, HP, BP };

    void prepare    (double sampleRate) noexcept;
    void reset      () noexcept;

    // --- Parameter setters (cheap; update coefficients lazily) ---
    void setType      (int t)     noexcept;
    void setCutoff    (float hz)  noexcept;
    void setResonance (float res) noexcept;  // 0 (clean) → 1 (near self-oscillation)
    void setDrive     (float d)   noexcept;  // 0 (off)   → 1 (full drive)

    float processLeft  (float in) noexcept;
    float processRight (float in) noexcept;

private:
    // ---- Parameters ----
    double sr        = 44100.0;
    Type   type      = Type::LP12;
    float  cutoff    = 1000.0f;
    float  resonance = 0.0f;
    float  drive     = 0.0f;

    // ---- Coefficients (recomputed when cutoff/res changes) ----
    float a1 = 0.0f, a2 = 0.0f, a3 = 0.0f, k = 0.0f;
    bool  needsCoeffUpdate = true;

    void updateCoefficients() noexcept;

    // ---- Per-channel state (stage 1 and stage 2 for LP24) ----
    struct ChanState
    {
        float s1_1 = 0.0f, s2_1 = 0.0f; // stage 1 integrator states
        float s1_2 = 0.0f, s2_2 = 0.0f; // stage 2 integrator states (LP24 only)
    };
    ChanState stateL, stateR;

    float processSample (float in, ChanState& st) noexcept;
};
