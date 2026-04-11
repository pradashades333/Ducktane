#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include "../Data/PresetManager.h"

/**
 * Full-editor overlay that mimics Massive's preset browser.
 *
 * Left sidebar  — category pill buttons (ALL / BASS / LEAD / PAD / KEYS /
 *                 STAB / SOUNDSCAPE / FX / ARP / OTHER)
 * Right area    — scrollable preset list, single-click loads, shows category
 *
 * Placement: setBounds(editor.getLocalBounds()) then setVisible(true/false).
 */
class BrowserPanel : public juce::Component,
                     public juce::ListBoxModel
{
public:
    explicit BrowserPanel (PresetManager& pm);

    void paint   (juce::Graphics&) override;
    void resized () override;

    /** Refresh preset list from disk and show the panel. */
    void refresh ();

    /** Called when the user closes the browser (bind in PluginEditor). */
    std::function<void()> onClose;

    /** Called when a preset is loaded from the list. */
    std::function<void()> onPresetLoaded;

private:
    PresetManager& presets;

    // ---- inner panel geometry (computed in resized) ----
    juce::Rectangle<int> panel;   // the white card area
    juce::Rectangle<int> sidebar; // left category column
    juce::Rectangle<int> listArea;// right preset list

    // ---- header ----
    juce::TextButton closeBtn { "X" };

    // ---- category sidebar ----
    juce::String selectedCategory { "ALL" };
    std::vector<std::unique_ptr<juce::TextButton>> catBtns;

    // ---- preset list ----
    juce::ListBox     presetList { {}, this };
    juce::StringArray filteredPresets;

    void selectCategory (const juce::String& cat);

    // ---- ListBoxModel ----
    int  getNumRows () override { return filteredPresets.size(); }
    void paintListBoxItem (int row, juce::Graphics&,
                           int w, int h, bool selected) override;
    void listBoxItemClicked        (int row, const juce::MouseEvent&) override;
    void listBoxItemDoubleClicked  (int row, const juce::MouseEvent&) override;
    juce::String getTooltipForRow  (int row) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BrowserPanel)
};
