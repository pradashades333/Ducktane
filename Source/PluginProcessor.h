#pragma once
#include <JuceHeader.h>
#include "DSP/Lfo.h"
#include "DSP/FxChain.h"

// ============================================================
// Lock-free ring buffer for oscilloscope display (audio → UI)
// ============================================================
struct ScopeBuffer
{
    static constexpr int kSize = 4096; // must be power of 2

    std::array<float, kSize> data {};
    std::atomic<int>   writeHead     { 0 };
    std::atomic<float> noteFrequency { 440.0f };
    std::atomic<float> sampleRate    { 44100.0f };
    std::atomic<bool>  isActive      { false };

    /** Called from audio thread — pushes `count` samples into the ring. */
    void push (const float* samples, int count) noexcept
    {
        int w = writeHead.load (std::memory_order_relaxed);
        for (int i = 0; i < count; ++i)
        {
            data[w & (kSize - 1)] = samples[i];
            ++w;
        }
        writeHead.store (w, std::memory_order_release);
    }

    /** Called from message thread.
     *  Finds the most recent rising zero-crossing, copies `outSize` samples
     *  into `out`, and returns outSize on success (or 0 if not enough data). */
    int readTriggered (float* out, int outSize) const noexcept
    {
        const int w = writeHead.load (std::memory_order_acquire);

        // We need outSize samples plus a search window for the trigger
        constexpr int kSearchWindow = 512;
        if (w < outSize + kSearchWindow)
            return 0;

        // Search backwards from (w - outSize) to find rising zero-crossing
        const int searchStart = w - outSize - kSearchWindow;
        int triggerPos = w - outSize; // fallback: most recent block

        for (int i = searchStart; i < w - outSize; ++i)
        {
            const float s0 = data[(i - 1) & (kSize - 1)];
            const float s1 = data[i       & (kSize - 1)];
            if (s0 < 0.0f && s1 >= 0.0f)
            {
                triggerPos = i;
                break;
            }
        }

        for (int i = 0; i < outSize; ++i)
            out[i] = data[(triggerPos + i) & (kSize - 1)];

        return outSize;
    }
};

class MassiveSynthAudioProcessor : public juce::AudioProcessor
{
public:
    MassiveSynthAudioProcessor();
    ~MassiveSynthAudioProcessor() override;

    // ---- AudioProcessor interface ----
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool   acceptsMidi()  const override { return true; }
    bool   producesMidi() const override { return false; }
    bool   isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int  getNumPrograms()    override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // ---- Public DSP members used by the editor ----
    juce::AudioProcessorValueTreeState apvts;

    /** Set to true each processBlock while any voice is sounding.
     *  Read by WaveformDisplay on the message thread — atomic for safety. */
    std::atomic<bool> isAudioActive { false };

    /** Lock-free ring buffer of post-FX audio samples for the oscilloscope UI. */
    ScopeBuffer scopeBuffer;

private:
    // ---- APVTS ----
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // ---- DSP ----
    juce::Synthesiser synth;   // polyphonic voice pool
    Lfo               lfo1, lfo2;
    FxChain           fxChain;

    // ---- Cached pointers for LFO parameters ----
    std::atomic<float>* pLfo1Shape;
    std::atomic<float>* pLfo1Rate;
    std::atomic<float>* pLfo1Sync;
    std::atomic<float>* pLfo2Shape;
    std::atomic<float>* pLfo2Rate;
    std::atomic<float>* pLfo2Sync;
    std::atomic<float>* pMasterVolume;

    // ---- Host tempo (for LFO sync, updated each block) ----
    double bpm = 120.0;

    static constexpr int kNumVoices = 16;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MassiveSynthAudioProcessor)
};
