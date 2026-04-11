#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

/**
 * One ADSR section — instantiated twice (Amp + Mod).
 * Pass the parameter prefix ("amp_" or "mod_") and a display title.
 */
class EnvelopePanel : public juce::Component
{
public:
    EnvelopePanel (juce::AudioProcessorValueTreeState& apvts,
                   const juce::String& prefix,
                   const juce::String& title);

    void paint  (juce::Graphics&) override;
    void resized() override;

private:
    juce::String title;

    juce::Slider attackKnob, decayKnob, sustainKnob, releaseKnob;
    juce::Label  attackLbl,  decayLbl,  sustainLbl,  releaseLbl;

    using SlAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SlAtt> attackAtt, decayAtt, sustainAtt, releaseAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopePanel)
};
