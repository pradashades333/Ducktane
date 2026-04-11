#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include <DucktaneAssets.h>

/** Filter section + sub oscillator controls. */
class FilterPanel : public juce::Component
{
public:
    explicit FilterPanel (juce::AudioProcessorValueTreeState& apvts);
    void paint  (juce::Graphics&) override;
    void resized() override;

private:
    int            subDivY  = 0;
    juce::Image    logoImage;
    juce::Rectangle<int> logoArea;

    juce::ComboBox filterTypeBox;
    juce::Slider   cutoffKnob, resKnob, driveKnob, envAmtKnob;
    juce::Label    cutoffLbl, resLbl, driveLbl, envAmtLbl;

    juce::Slider   subLevelKnob;
    juce::Label    subLevelLbl;
    juce::ComboBox subOctaveBox;

    using SlAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CbAtt = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<CbAtt> filterTypeAtt, subOctaveAtt;
    std::unique_ptr<SlAtt> cutoffAtt, resAtt, driveAtt, envAmtAtt, subLevelAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterPanel)
};
