#include "PresetPanel.h"

PresetPanel::PresetPanel (juce::AudioProcessorValueTreeState& apvts,
                          PresetManager& pm)
    : presets (pm)
{
    addAndMakeVisible (prevBtn);
    addAndMakeVisible (nextBtn);
    addAndMakeVisible (saveBtn);
    addAndMakeVisible (browseBtn);
    addAndMakeVisible (importBtn);
    addAndMakeVisible (presetBox);

    presetBox.setTextWhenNoChoicesAvailable ("-- No Presets --");

    prevBtn.onClick = [this]
    {
        presets.previousPreset();
        refreshList();
    };

    nextBtn.onClick = [this]
    {
        presets.nextPreset();
        refreshList();
    };

    saveBtn.onClick   = [this] { onSave(); };
    browseBtn.onClick = [this] { if (onBrowseClicked) onBrowseClicked(); };

    importBtn.onClick = [this]
    {
        auto chooser = std::make_shared<juce::FileChooser> (
            "Import Ducktane Preset", juce::File{}, "*.xml");

        chooser->launchAsync (juce::FileBrowserComponent::openMode |
                              juce::FileBrowserComponent::canSelectFiles,
            [this, chooser] (const juce::FileChooser& fc)
            {
                const auto result = fc.getResult();
                if (result.existsAsFile())
                {
                    presets.importPreset (result);
                    refreshList();
                }
            });
    };

    presetBox.onChange = [this] { onPresetSelected(); };

    // Master volume
    MSLayout::setupKnob (*this, masterVolKnob, masterVolLbl, "VOL");
    masterVolAtt = std::make_unique<SlAtt> (apvts, "master_volume", masterVolKnob);

    refreshList();
}

void PresetPanel::refreshList()
{
    presets.refreshPresetList();
    presetBox.clear (juce::dontSendNotification);
    int i = 1;
    for (const auto& name : presets.getPresetNames())
        presetBox.addItem (name, i++);

    const int idx = presets.getPresetNames().indexOf (presets.getCurrentPresetName());
    presetBox.setSelectedItemIndex (idx >= 0 ? idx : 0, juce::dontSendNotification);
}

void PresetPanel::onPresetSelected()
{
    const int idx = presetBox.getSelectedItemIndex();
    if (idx >= 0 && idx < presets.getPresetNames().size())
        presets.loadPreset (presets.getPresetNames()[idx]);
}

void PresetPanel::onSave()
{
    auto* aw = new juce::AlertWindow ("Save Preset", "Enter a preset name:",
                                      juce::AlertWindow::NoIcon);
    aw->addTextEditor ("name",
                       presets.getCurrentPresetName().isEmpty()
                           ? "My Preset" : presets.getCurrentPresetName());

    // Category picker
    const auto cats = PresetManager::getCategories();
    aw->addComboBox ("category", cats, "Category");
    if (auto* cb = aw->getComboBoxComponent ("category"))
    {
        // Default to current preset's existing category if any
        const juce::String existingCat = presets.getCategory (presets.getCurrentPresetName());
        const int idx = cats.indexOf (existingCat);
        cb->setSelectedItemIndex (idx >= 0 ? idx : 0, juce::dontSendNotification);
    }

    aw->addButton ("Save",   1, juce::KeyPress (juce::KeyPress::returnKey));
    aw->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    aw->enterModalState (true,
        juce::ModalCallbackFunction::create ([this, aw] (int result)
        {
            if (result == 1)
            {
                const juce::String name = aw->getTextEditorContents ("name").trim();
                juce::String cat;
                if (auto* cb = aw->getComboBoxComponent ("category"))
                    cat = cb->getText();

                if (name.isNotEmpty())
                {
                    presets.savePreset (name, cat == "ALL" ? juce::String{} : cat);
                    refreshList();
                }
            }
        }), true);
}

void PresetPanel::paint (juce::Graphics& g)
{
    g.setColour (MSColors::panelBg);
    g.fillRect (getLocalBounds());
    g.setColour (MSColors::border);
    g.drawLine (0.0f, static_cast<float> (getHeight()) - 1.0f,
                static_cast<float> (getWidth()), static_cast<float> (getHeight()) - 1.0f, 1.0f);

    // Title
    g.setColour (MSColors::accentHi);
    g.setFont (juce::Font (16.0f, juce::Font::bold));
    g.drawText ("DUCKTANE", getLocalBounds().withWidth (200).translated (8, 0),
                juce::Justification::centredLeft);
}

void PresetPanel::resized()
{
    auto area = getLocalBounds().reduced (8, 6);

    // Master volume knob on the far right
    auto volArea = area.removeFromRight (52);
    MSLayout::placeKnob (masterVolKnob, masterVolLbl, volArea);

    area.removeFromLeft (160); // space for the "DUCKTANE" title

    // Right side: IMPORT | BROWSE | SAVE | > | <
    importBtn.setBounds (area.removeFromRight (62).reduced (0, 6));
    area.removeFromRight (4);
    browseBtn.setBounds (area.removeFromRight (68).reduced (0, 6));
    area.removeFromRight (4);
    saveBtn.setBounds   (area.removeFromRight (52).reduced (0, 6));
    area.removeFromRight (4);
    nextBtn.setBounds   (area.removeFromRight (26).reduced (0, 6));
    prevBtn.setBounds   (area.removeFromRight (26).reduced (0, 6));
    area.removeFromRight (4);
    presetBox.setBounds (area.reduced (0, 6));
}
