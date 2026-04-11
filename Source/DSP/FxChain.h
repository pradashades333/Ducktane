#pragma once
#include <JuceHeader.h>
#include <vector>

/**
 * Post-synth stereo FX chain:
 *   Distortion (tanh soft-clip) → Chorus (modulated delay)
 *       → Feedback Delay → Reverb (Freeverb)
 *
 * Each stage has a mix parameter (0 = dry/bypassed, 1 = full wet).
 * All APVTS pointers are cached at construction.
 */
class FxChain
{
public:
    explicit FxChain (juce::AudioProcessorValueTreeState& apvts);

    void prepare (double sampleRate, int samplesPerBlock);
    void process (juce::AudioBuffer<float>& buffer);

private:
    // ---- Cached APVTS parameter pointers ----
    std::atomic<float>* pDistDrive;
    std::atomic<float>* pDistMix;
    std::atomic<float>* pPhaserRate;
    std::atomic<float>* pPhaserDepth;
    std::atomic<float>* pPhaserMix;
    std::atomic<float>* pCrushBits;
    std::atomic<float>* pCrushMix;
    std::atomic<float>* pChorusRate;
    std::atomic<float>* pChorusDepth;
    std::atomic<float>* pChorusMix;
    std::atomic<float>* pDelayTime;
    std::atomic<float>* pDelayFeedback;
    std::atomic<float>* pDelayMix;
    std::atomic<float>* pReverbRoom;
    std::atomic<float>* pReverbDamp;
    std::atomic<float>* pReverbMix;

    // ---- State ----
    double sampleRate = 44100.0;

    // Chorus — two independent delay lines, L 0° / R 90°
    static constexpr int kChorusBufSize = 8192; // > 100 ms at any sample rate
    std::vector<float> chorusBufL, chorusBufR;
    int    chorusWritePos = 0;
    double chorusPhase    = 0.0; // L phase [0,1); R = L + 0.25

    // Feedback Delay
    // Allocated for 2 s at 192 kHz to cover any reasonable SR/time.
    static constexpr int kMaxDelaySamples = 192000 * 2;
    std::vector<float> delayBufL, delayBufR;
    int delayWritePos = 0;

    // Reverb (JUCE Freeverb wrapper)
    juce::dsp::Reverb reverb;

    // ---- Phaser state (4-stage allpass per channel) ----
    struct AllpassStage { float x1 = 0.f, y1 = 0.f; };
    std::array<AllpassStage, 4> phaserL {}, phaserR {};
    double phaserPhase   = 0.0;
    float  phaserFbL     = 0.0f, phaserFbR = 0.0f;

    // ---- Bitcrusher state (hold-sample SRR) ----
    float crushHoldL = 0.f, crushHoldR = 0.f;
    int   crushCounter = 0;

    // ---- Processing helpers ----
    void processDistortion (juce::AudioBuffer<float>&,  float drive, float mix) noexcept;
    void processPhaser     (juce::AudioBuffer<float>&,  float rate, float depth, float mix) noexcept;
    void processChorus     (juce::AudioBuffer<float>&,  float rate, float depth, float mix) noexcept;
    void processDelay      (juce::AudioBuffer<float>&,  float timeMs, float feedback, float mix) noexcept;
    void processReverb     (juce::AudioBuffer<float>&,  float room, float damp, float mix) noexcept;
    void processBitcrusher (juce::AudioBuffer<float>&,  float bits, float mix) noexcept;

    static float readInterp (const std::vector<float>& buf, int writePos, float delaySamples) noexcept;
};
