#pragma once
#include <JuceHeader.h>

/**
 * Saves and loads APVTS state as XML files in the user's AppData folder.
 * Presets live at:  %APPDATA%/MassiveSynth/Presets/<name>.xml  (Windows)
 *                   ~/Library/Application Support/MassiveSynth/Presets/<name>.xml  (macOS)
 */
class PresetManager
{
public:
    static constexpr const char* kExtension = ".xml";

    explicit PresetManager (juce::AudioProcessorValueTreeState& apvts);

    // --- Scanning ---
    void      refreshPresetList();
    juce::StringArray getPresetNames() const { return presetNames; }

    // --- Load / Save ---
    bool loadPreset   (const juce::String& name);
    /** Saves current state with an optional category tag. */
    bool savePreset   (const juce::String& name,
                       const juce::String& category = {});
    void deletePreset  (const juce::String& name);
    /** Copies an external XML file into the presets folder. */
    bool importPreset  (const juce::File& sourceFile);

    juce::String getCurrentPresetName() const { return currentPreset; }

    /** Writes factory presets to disk only when the folder is empty. */
    void installFactoryPresetsIfNeeded();

    // --- Categories ---
    static juce::StringArray  getCategories();
    juce::String              getCategory (const juce::String& name) const;
    juce::StringArray         getPresetsByCategory (const juce::String& category) const;

    // --- Navigation ---
    juce::String nextPreset();
    juce::String previousPreset();

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::File presetsDirectory;
    juce::StringArray presetNames;
    juce::String currentPreset;

    juce::File presetFile (const juce::String& name) const;
};
