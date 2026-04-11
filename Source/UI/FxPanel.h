#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

/** Full-width FX strip: Distortion | Phaser | Chorus | Delay | Reverb | Bitcrusher */
class FxPanel : public juce::Component
{
public:
    explicit FxPanel (juce::AudioProcessorValueTreeState& apvts);
    void paint  (juce::Graphics&) override;
    void resized() override;

private:
    // Distortion
    juce::Slider distDriveKnob, distMixKnob;
    juce::Label  distDriveLbl,  distMixLbl;

    // Phaser
    juce::Slider phaRateKnob, phaDepthKnob, phaMixKnob;
    juce::Label  phaRateLbl,  phaDepthLbl,  phaMixLbl;

    // Chorus
    juce::Slider chorRateKnob, chorDepthKnob, chorMixKnob;
    juce::Label  chorRateLbl,  chorDepthLbl,  chorMixLbl;

    // Delay
    juce::Slider delayTimeKnob, delayFbKnob, delayMixKnob;
    juce::Label  delayTimeLbl,  delayFbLbl,  delayMixLbl;

    // Reverb
    juce::Slider revRoomKnob, revDampKnob, revMixKnob;
    juce::Label  revRoomLbl,  revDampLbl,  revMixLbl;

    // Bitcrusher
    juce::Slider crushBitsKnob, crushMixKnob;
    juce::Label  crushBitsLbl,  crushMixLbl;

    using SlAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SlAtt>
        distDriveAtt, distMixAtt,
        phaRateAtt, phaDepthAtt, phaMixAtt,
        chorRateAtt, chorDepthAtt, chorMixAtt,
        delayTimeAtt, delayFbAtt, delayMixAtt,
        revRoomAtt, revDampAtt, revMixAtt,
        crushBitsAtt, crushMixAtt;

    void drawFxSection (juce::Graphics&, juce::Rectangle<int>, const juce::String&) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxPanel)
};
