#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>

/**
 * Virtual-analog oscillator supporting Sine, Triangle, Saw and Square waveforms
 * with polyBLEP anti-aliasing.  Up to kMaxUnison=7 detuned voices are spread
 * across the stereo field using constant-power panning.
 *
 * Usage pattern (once per block before the per-sample loop):
 *   osc.setWaveform(...); osc.setFrequency(...); osc.setUnison...();
 *   osc.setPitchOffsetSemitones(...);
 *   // then per sample:
 *   auto [L, R] = osc.processSample();
 */
class Oscillator
{
public:
    enum class Waveform { Sine = 0, Triangle, Saw, Square };
    static constexpr int kMaxUnison = 7;

    void prepare (double sampleRate) noexcept;

    // --- Setters (flag a recompute; cheap per block) ---
    void setWaveform            (int wf)       noexcept;
    void setFrequency           (float hz)     noexcept;
    void setUnisonVoices        (int v)        noexcept;
    void setUnisonDetune        (float cents)  noexcept;
    void setUnisonSpread        (float s)      noexcept;  // 0=mono, 1=full stereo
    void setPitchOffsetSemitones(float semi)   noexcept;

    /** Randomise all voice phases — call on note start. */
    void reset() noexcept;

    /** Returns stereo {left, right} for one sample. */
    std::array<float, 2> processSample() noexcept;

private:
    // ---- Parameters ----
    double   sampleRate           = 44100.0;
    float    baseFrequency        = 440.0f;
    float    pitchOffsetSemitones = 0.0f;
    Waveform waveform             = Waveform::Saw;
    int      voices               = 1;
    float    unisonDetune         = 0.0f;
    float    unisonSpread         = 0.5f;

    // ---- Per-voice pre-computed data ----
    struct VoiceData
    {
        float phaseInc = 0.0f; // phase increment per sample
        float panL     = 1.0f;
        float panR     = 1.0f;
    };
    std::array<VoiceData, kMaxUnison> voiceData;
    std::array<double,   kMaxUnison> phases{};
    bool needsUpdate = true;

    void updateVoiceData() noexcept;

    // ---- Sample generation ----
    float generateSample (Waveform wf, float phase, float dt) const noexcept;
    static float polyBlep (float phase, float dt) noexcept;
};
