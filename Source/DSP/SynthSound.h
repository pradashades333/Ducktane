#pragma once
#include <JuceHeader.h>

// A simple MPE/polyphonic sound descriptor — no data needed.
struct SynthSound : public juce::SynthesiserSound
{
    bool appliesToNote    (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};
