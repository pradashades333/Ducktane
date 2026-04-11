#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include "../PluginProcessor.h"  // for ScopeBuffer

/**
 * Real-time oscilloscope display for one oscillator channel.
 *
 * Reads post-FX audio samples from the processor's ScopeBuffer via a
 * zero-crossing trigger (rising edge), so the waveform stays stable
 * like a hardware oscilloscope.  Renders at 60 Hz.
 *
 * Fallback
 * --------
 * When no ScopeBuffer is set (or before first audio arrives) the display
 * shows a flat centre line, consistent with silence.
 *
 * Usage
 * -----
 *   display.setScopeBuffer (&processor.scopeBuffer);   // call once after ctor
 */
class WaveformDisplay : public juce::Component,
                        public juce::Timer
{
public:
    WaveformDisplay();
    ~WaveformDisplay() override;

    /** Point the display at the processor's ring buffer. */
    void setScopeBuffer (ScopeBuffer* buf) noexcept { pScope = buf; }

    // Legacy shim so PluginEditor doesn't need changing yet
    void setActiveFlag (const std::atomic<bool>*) noexcept {}

    // ---- Component ----
    void paint (juce::Graphics&) override;

    // ---- Timer: 60 Hz repaint ----
    void timerCallback() override;

private:
    ScopeBuffer* pScope = nullptr;

    // Scratch buffer for one display frame of samples
    static constexpr int kDisplaySamples = 512;
    std::array<float, kDisplaySamples> frameBuf {};
    bool frameValid = false;

    // Glow helper
    static void strokeGlow (juce::Graphics&, const juce::Path&,
                            juce::Colour, float alpha, float thickness);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};
