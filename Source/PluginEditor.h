#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/CustomLookAndFeel.h"
#include "UI/PresetPanel.h"
#include "UI/OscillatorPanel.h"
#include "UI/FilterPanel.h"
#include "UI/EnvelopePanel.h"
#include "UI/LfoPanel.h"
#include "UI/FxPanel.h"
#include "UI/BrowserPanel.h"
#include "Data/PresetManager.h"

class MassiveSynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit MassiveSynthAudioProcessorEditor (MassiveSynthAudioProcessor&);
    ~MassiveSynthAudioProcessorEditor() override;

    void paint  (juce::Graphics&) override;
    void resized() override;

private:
    MassiveSynthAudioProcessor& audioProcessor;

    CustomLookAndFeel laf;

    PresetManager    presetManager;
    PresetPanel      presetPanel;
    OscillatorPanel  osc1Panel, osc2Panel;
    FilterPanel      filterPanel;
    EnvelopePanel    ampEnvPanel, modEnvPanel;
    LfoPanel         lfo1Panel, lfo2Panel;
    FxPanel          fxPanel;
    BrowserPanel     browserPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MassiveSynthAudioProcessorEditor)
};
