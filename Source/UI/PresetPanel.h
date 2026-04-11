#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include "../Data/PresetManager.h"

class PresetPanel : public juce::Component
{
public:
    PresetPanel (juce::AudioProcessorValueTreeState& apvts,
                 PresetManager& presetManager);

    void paint  (juce::Graphics&) override;
    void resized() override;

    void refreshList();

    /** Assign this to open the browser overlay. */
    std::function<void()> onBrowseClicked;

private:
    PresetManager& presets;

    juce::TextButton prevBtn    { "<" };
    juce::TextButton nextBtn    { ">" };
    juce::TextButton saveBtn    { "SAVE" };
    juce::TextButton browseBtn  { "BROWSE" };
    juce::TextButton importBtn  { "IMPORT" };
    juce::ComboBox   presetBox;

    // Master volume knob (lives in the preset bar for easy access)
    juce::Slider masterVolKnob;
    juce::Label  masterVolLbl;

    using SlAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SlAtt> masterVolAtt;

    void onPresetSelected();
    void onSave();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetPanel)
};
