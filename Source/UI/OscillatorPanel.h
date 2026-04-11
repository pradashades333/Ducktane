#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include "WaveformDisplay.h"

/**
 * Displays all controls for one oscillator (OSC 1 or OSC 2).
 * Construct with the APVTS and the parameter ID prefix ("osc1_" or "osc2_").
 */
class OscillatorPanel : public juce::Component
{
public:
    OscillatorPanel (juce::AudioProcessorValueTreeState& apvts,
                     const juce::String& prefix,
                     const juce::String& title);

    void paint  (juce::Graphics&) override;
    void resized() override;

    WaveformDisplay& getWaveDisplay() noexcept { return waveDisplay; }

private:
    juce::String title;
    int unisonDivY = 200;

    WaveformDisplay waveDisplay {};

    // Controls
    juce::ComboBox shapeBox;
    juce::Slider   octaveKnob, semiKnob, fineKnob, levelKnob;
    juce::Slider   voicesKnob, detuneKnob, spreadKnob;
    juce::Label    octaveLbl, semiLbl, fineLbl, levelLbl;
    juce::Label    voicesLbl, detuneLbl, spreadLbl;

    // Attachments
    using SlAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CbAtt = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<CbAtt> shapeAtt;
    std::unique_ptr<SlAtt> octaveAtt, semiAtt, fineAtt, levelAtt;
    std::unique_ptr<SlAtt> voicesAtt, detuneAtt, spreadAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscillatorPanel)
};
