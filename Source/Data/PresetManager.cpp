#include "PresetManager.h"

PresetManager::PresetManager (juce::AudioProcessorValueTreeState& apvts_)
    : apvts (apvts_)
{
    presetsDirectory = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                           .getChildFile ("Ducktane")
                           .getChildFile ("Presets");

    if (! presetsDirectory.exists())
        presetsDirectory.createDirectory();

    refreshPresetList();
    installFactoryPresetsIfNeeded();
}

void PresetManager::refreshPresetList()
{
    presetNames.clear();
    for (auto& f : presetsDirectory.findChildFiles (juce::File::findFiles, false, "*.xml"))
        presetNames.add (f.getFileNameWithoutExtension());
    presetNames.sort (true);
}

juce::File PresetManager::presetFile (const juce::String& name) const
{
    return presetsDirectory.getChildFile (name + kExtension);
}

bool PresetManager::loadPreset (const juce::String& name)
{
    const juce::File f = presetFile (name);
    if (! f.existsAsFile()) return false;

    if (auto xml = juce::XmlDocument::parse (f))
    {
        if (xml->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
            currentPreset = name;
            return true;
        }
    }
    return false;
}

bool PresetManager::savePreset (const juce::String& name, const juce::String& category)
{
    if (name.isEmpty()) return false;

    auto state = apvts.copyState();
    if (auto xml = state.createXml())
    {
        if (category.isNotEmpty())
            xml->setAttribute ("presetCategory", category);

        if (xml->writeTo (presetFile (name)))
        {
            currentPreset = name;
            refreshPresetList();
            return true;
        }
    }
    return false;
}

void PresetManager::installFactoryPresetsIfNeeded()
{
    if (! presetNames.isEmpty()) return; // user already has presets

    // Helper: set a float parameter to a real-world value (non-normalised)
    auto setF = [&] (const char* id, float val)
    {
        if (auto* p = dynamic_cast<juce::AudioParameterFloat*> (apvts.getParameter (id)))
            *p = val;
    };
    auto setI = [&] (const char* id, int val)
    {
        if (auto* p = dynamic_cast<juce::AudioParameterInt*> (apvts.getParameter (id)))
            *p = val;
    };
    auto setC = [&] (const char* id, int idx)
    {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (id)))
            *p = idx;
    };
    auto reset = [&]
    {
        // Silence OSC2 + sub, reset filter, default envs
        setF ("osc1_level", 0.8f);  setC ("osc1_shape", 2); // Saw
        setI ("osc1_octave", 0);    setI ("osc1_semi", 0);
        setI ("osc1_unison_voices", 1); setF ("osc1_unison_detune", 20.f);
        setF ("osc2_level", 0.0f);
        setF ("sub_level",  0.0f);
        setF ("filter_cutoff", 8000.f); setF ("filter_res", 0.0f);
        setF ("filter_env_amt", 0.0f);
        setF ("amp_attack", 0.005f); setF ("amp_decay", 0.2f);
        setF ("amp_sustain", 0.7f);  setF ("amp_release", 0.4f);
        setF ("mod_attack", 0.01f);  setF ("mod_decay", 0.5f);
        setF ("mod_sustain", 0.0f);  setF ("mod_release", 0.2f);
        setF ("lfo1_depth", 0.0f);   setF ("lfo2_depth", 0.0f);
        setF ("dist_mix", 0.0f);     setF ("phaser_mix", 0.0f);
        setF ("chorus_mix", 0.0f);   setF ("delay_mix", 0.0f);
        setF ("reverb_mix", 0.0f);   setF ("crush_mix", 0.0f);
        setF ("master_volume", 0.7f);
    };

    // ---- 1. Supersaw Lead ----
    reset();
    setC ("osc1_shape", 2); // Saw
    setI ("osc1_unison_voices", 7);
    setF ("osc1_unison_detune", 30.f);
    setF ("osc1_unison_spread", 0.8f);
    setF ("filter_cutoff", 6000.f); setF ("filter_res", 0.15f);
    setF ("amp_attack", 0.02f); setF ("amp_release", 0.5f);
    setF ("chorus_rate", 1.2f); setF ("chorus_depth", 0.4f); setF ("chorus_mix", 0.3f);
    setF ("reverb_room", 0.4f); setF ("reverb_damp", 0.5f);  setF ("reverb_mix", 0.2f);
    savePreset ("001 Supersaw Lead", "LEAD");

    // ---- 2. Sub Bass ----
    reset();
    setC ("osc1_shape", 0); // Sine
    setI ("osc1_octave", -1);
    setF ("osc1_level", 1.0f);
    setF ("sub_level", 0.5f);
    setF ("filter_cutoff", 300.f); setF ("filter_res", 0.1f);
    setF ("amp_attack", 0.001f); setF ("amp_decay", 0.15f);
    setF ("amp_sustain", 0.6f);  setF ("amp_release", 0.15f);
    savePreset ("002 Sub Bass", "BASS");

    // ---- 3. Reese Bass ----
    reset();
    setC ("osc1_shape", 2); // Saw
    setC ("osc2_shape", 2);
    setI ("osc1_octave", -1);  setI ("osc2_octave", -1);
    setF ("osc2_level", 0.7f); setI ("osc2_semi", 0);
    setF ("osc1_fine", -8.f);  setF ("osc2_fine", 8.f);
    setF ("filter_cutoff", 400.f); setF ("filter_res", 0.4f);
    setF ("filter_env_amt", 0.3f);
    setF ("amp_attack", 0.002f); setF ("amp_release", 0.25f);
    setF ("chorus_mix", 0.2f);
    savePreset ("003 Reese Bass", "BASS");

    // ---- 4. Warm Pad ----
    reset();
    setC ("osc1_shape", 0); // Sine
    setC ("osc2_shape", 1); // Triangle
    setF ("osc2_level", 0.5f); setI ("osc2_semi", 7); // 5th above
    setI ("osc1_unison_voices", 3);
    setF ("osc1_unison_detune", 12.f); setF ("osc1_unison_spread", 0.6f);
    setF ("filter_cutoff", 2500.f); setF ("filter_res", 0.05f);
    setF ("amp_attack", 0.4f); setF ("amp_decay", 0.3f);
    setF ("amp_sustain", 0.8f); setF ("amp_release", 1.2f);
    setF ("reverb_room", 0.7f); setF ("reverb_damp", 0.3f); setF ("reverb_mix", 0.5f);
    setF ("chorus_mix", 0.25f);
    savePreset ("004 Warm Pad", "PAD");

    // ---- 5. Pluck Keys ----
    reset();
    setC ("osc1_shape", 2); // Saw
    setF ("filter_cutoff", 5000.f); setF ("filter_res", 0.2f);
    setF ("filter_env_amt", 0.6f);
    setF ("amp_attack", 0.001f); setF ("amp_decay", 0.4f);
    setF ("amp_sustain", 0.0f);  setF ("amp_release", 0.3f);
    setF ("mod_attack", 0.001f); setF ("mod_decay", 0.3f);
    setF ("reverb_room", 0.35f); setF ("reverb_mix", 0.15f);
    savePreset ("005 Pluck Keys", "KEYS");

    // ---- 6. Stab ----
    reset();
    setC ("osc1_shape", 3); // Square
    setC ("osc2_shape", 2); // Saw
    setF ("osc2_level", 0.5f); setI ("osc2_semi", 12); // octave up
    setF ("filter_cutoff", 3000.f); setF ("filter_res", 0.5f);
    setF ("filter_env_amt", 0.8f);
    setF ("amp_attack", 0.001f); setF ("amp_decay", 0.12f);
    setF ("amp_sustain", 0.0f);  setF ("amp_release", 0.1f);
    setF ("mod_attack", 0.001f); setF ("mod_decay", 0.1f);
    setF ("dist_drive", 0.3f);   setF ("dist_mix", 0.4f);
    savePreset ("006 Stabby", "STAB");

    // ---- 7. Atmospheric Pad ----
    reset();
    setC ("osc1_shape", 0); // Sine
    setI ("osc1_unison_voices", 5);
    setF ("osc1_unison_detune", 25.f); setF ("osc1_unison_spread", 1.0f);
    setF ("filter_cutoff", 1800.f); setF ("filter_res", 0.0f);
    setF ("amp_attack", 1.5f); setF ("amp_decay", 0.5f);
    setF ("amp_sustain", 0.9f); setF ("amp_release", 2.5f);
    setF ("reverb_room", 0.9f); setF ("reverb_damp", 0.2f); setF ("reverb_mix", 0.7f);
    setF ("phaser_rate", 0.12f); setF ("phaser_depth", 0.6f); setF ("phaser_mix", 0.4f);
    savePreset ("007 Atmospheric", "SOUNDSCAPE");

    // ---- 8. Lo-Fi Keys ----
    reset();
    setC ("osc1_shape", 1); // Triangle
    setF ("filter_cutoff", 3500.f); setF ("filter_res", 0.1f);
    setF ("amp_attack", 0.003f); setF ("amp_decay", 0.5f);
    setF ("amp_sustain", 0.2f);  setF ("amp_release", 0.4f);
    setF ("crush_bits", 8.0f);   setF ("crush_mix", 0.6f);
    setF ("reverb_room", 0.3f);  setF ("reverb_mix", 0.2f);
    savePreset ("008 Lo-Fi Keys", "KEYS");

    refreshPresetList();
}

juce::StringArray PresetManager::getCategories()
{
    return { "ALL", "BASS", "LEAD", "PAD", "KEYS", "STAB", "SOUNDSCAPE", "FX", "ARP", "OTHER" };
}

juce::String PresetManager::getCategory (const juce::String& name) const
{
    const juce::File f = presetFile (name);
    if (! f.existsAsFile()) return {};
    if (auto xml = juce::XmlDocument::parse (f))
        return xml->getStringAttribute ("presetCategory");
    return {};
}

juce::StringArray PresetManager::getPresetsByCategory (const juce::String& category) const
{
    if (category.isEmpty() || category == "ALL")
        return presetNames;

    juce::StringArray result;
    for (const auto& n : presetNames)
        if (getCategory (n).equalsIgnoreCase (category))
            result.add (n);
    return result;
}

bool PresetManager::importPreset (const juce::File& sourceFile)
{
    if (! sourceFile.existsAsFile()) return false;
    const juce::File dest = presetFile (sourceFile.getFileNameWithoutExtension());
    if (sourceFile.copyFileTo (dest))
    {
        refreshPresetList();
        return true;
    }
    return false;
}

void PresetManager::deletePreset (const juce::String& name)
{
    presetFile (name).deleteFile();
    if (currentPreset == name) currentPreset.clear();
    refreshPresetList();
}

juce::String PresetManager::nextPreset()
{
    if (presetNames.isEmpty()) return {};
    int idx = presetNames.indexOf (currentPreset);
    idx = (idx + 1) % presetNames.size();
    loadPreset (presetNames[idx]);
    return currentPreset;
}

juce::String PresetManager::previousPreset()
{
    if (presetNames.isEmpty()) return {};
    int idx = presetNames.indexOf (currentPreset);
    if (idx <= 0) idx = presetNames.size();
    loadPreset (presetNames[idx - 1]);
    return currentPreset;
}
