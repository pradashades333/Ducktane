#pragma once
#include <JuceHeader.h>
#include "Oscillator.h"
#include "StateVariableFilter.h"

/**
 * One polyphonic voice: 2x Oscillator + Sub oscillator →
 * State Variable Filter → Amplitude envelope.
 *
 * A separate mod-envelope shapes the filter cutoff per sample.
 * LFO values are supplied per-block by the processor.
 */
class SynthVoice : public juce::SynthesiserVoice
{
public:
    explicit SynthVoice (juce::AudioProcessorValueTreeState& apvts);

    // --- juce::SynthesiserVoice interface ---
    bool canPlaySound (juce::SynthesiserSound*) override;
    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound*, int pitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int newValue) override;
    void controllerMoved (int, int) override {}
    void renderNextBlock (juce::AudioBuffer<float>&, int startSample, int numSamples) override;

    // --- Called by PluginProcessor before renderNextBlock ---
    void prepareToPlay (double sampleRate, int samplesPerBlock, int numOutputChannels);

    /** Returns the current note frequency (set when the note starts). */
    float getCurrentFrequency() const noexcept { return currentFreq; }

    /** Processor supplies the current LFO values (per block). */
    void setModulationValues (float lfo1Val, float lfo2Val) noexcept
    {
        lfo1Value = lfo1Val;
        lfo2Value = lfo2Val;
    }

private:
    // ---- DSP objects ----
    Oscillator          osc1, osc2, subOsc;
    StateVariableFilter svFilter;
    juce::ADSR          ampEnv, modEnv;

    // ---- Voice state ----
    double sampleRate      = 44100.0;
    float  currentFreq     = 440.0f;
    float  currentVelocity = 1.0f;
    float  pitchWheelSemis = 0.0f;
    float  lfo1Value       = 0.0f;
    float  lfo2Value       = 0.0f;
    bool   isPrepared      = false;

    // ---- Cached APVTS parameter pointers (set in constructor) ----
    // OSC 1
    std::atomic<float>* pOsc1Shape;
    std::atomic<float>* pOsc1Octave;
    std::atomic<float>* pOsc1Semi;
    std::atomic<float>* pOsc1Fine;
    std::atomic<float>* pOsc1Level;
    std::atomic<float>* pOsc1UniVoices;
    std::atomic<float>* pOsc1UniDetune;
    std::atomic<float>* pOsc1UniSpread;
    // OSC 2
    std::atomic<float>* pOsc2Shape;
    std::atomic<float>* pOsc2Octave;
    std::atomic<float>* pOsc2Semi;
    std::atomic<float>* pOsc2Fine;
    std::atomic<float>* pOsc2Level;
    std::atomic<float>* pOsc2UniVoices;
    std::atomic<float>* pOsc2UniDetune;
    std::atomic<float>* pOsc2UniSpread;
    // Sub
    std::atomic<float>* pSubLevel;
    std::atomic<float>* pSubOctave;
    // Filter
    std::atomic<float>* pFilterType;
    std::atomic<float>* pFilterCutoff;
    std::atomic<float>* pFilterRes;
    std::atomic<float>* pFilterDrive;
    std::atomic<float>* pFilterEnvAmt;
    // Amp ADSR
    std::atomic<float>* pAmpAttack;
    std::atomic<float>* pAmpDecay;
    std::atomic<float>* pAmpSustain;
    std::atomic<float>* pAmpRelease;
    // Mod ADSR
    std::atomic<float>* pModAttack;
    std::atomic<float>* pModDecay;
    std::atomic<float>* pModSustain;
    std::atomic<float>* pModRelease;
    // LFO targets / depths
    std::atomic<float>* pLfo1Target;
    std::atomic<float>* pLfo1Depth;
    std::atomic<float>* pLfo2Target;
    std::atomic<float>* pLfo2Depth;

    // ---- Private helpers ----
    void updateOscillators  (float pitchLfoSemis) noexcept;
    void updateEnvelopes    () noexcept;
};
