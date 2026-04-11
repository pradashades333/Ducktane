#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

/**
 * One LFO section — instantiated twice (LFO 1 + LFO 2).
 * Pass the parameter prefix ("lfo1_" or "lfo2_") and a title.
 */
class LfoPanel : public juce::Component
{
public:
    LfoPanel (juce::AudioProcessorValueTreeState& apvts,
              const juce::String& prefix,
              const juce::String& title);

    void paint  (juce::Graphics&) override;
    void resized() override;

private:
    juce::String title;

    juce::ComboBox shapeBox, targetBox;
    juce::Slider   rateKnob, depthKnob;
    juce::Label    rateLbl, depthLbl;

    using SlAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CbAtt = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<CbAtt> shapeAtt, targetAtt;
    std::unique_ptr<SlAtt> rateAtt, depthAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LfoPanel)
};
