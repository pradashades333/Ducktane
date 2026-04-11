#include "BrowserPanel.h"

static const juce::StringArray kCategories =
    PresetManager::getCategories(); // ALL, BASS, LEAD, …

// ============================================================
BrowserPanel::BrowserPanel (PresetManager& pm) : presets (pm)
{
    // Close button
    addAndMakeVisible (closeBtn);
    closeBtn.onClick = [this] { if (onClose) onClose(); };

    // Category buttons — one per category, radio-group so only one active
    for (int i = 0; i < kCategories.size(); ++i)
    {
        auto btn = std::make_unique<juce::TextButton> (kCategories[i]);
        btn->setRadioGroupId (42);
        btn->setClickingTogglesState (true);
        btn->setToggleState (kCategories[i] == selectedCategory,
                             juce::dontSendNotification);
        btn->onClick = [this, cat = kCategories[i]] { selectCategory (cat); };

        // Colour the toggle-on state with accent
        btn->setColour (juce::TextButton::buttonOnColourId,  MSColors::accent);
        btn->setColour (juce::TextButton::textColourOnId,    juce::Colours::white);
        btn->setColour (juce::TextButton::buttonColourId,    MSColors::panelAlt);
        btn->setColour (juce::TextButton::textColourOffId,   MSColors::textMid);

        addAndMakeVisible (*btn);
        catBtns.push_back (std::move (btn));
    }

    // Preset list
    presetList.setRowHeight (26);
    presetList.setColour (juce::ListBox::backgroundColourId, juce::Colour (0xff0d0f1a));
    presetList.setColour (juce::ListBox::outlineColourId,    MSColors::border);
    presetList.setOutlineThickness (1);
    addAndMakeVisible (presetList);

    refresh();
}

// ============================================================
void BrowserPanel::refresh ()
{
    presets.refreshPresetList();
    selectCategory (selectedCategory);
}

void BrowserPanel::selectCategory (const juce::String& cat)
{
    selectedCategory = cat;
    filteredPresets  = presets.getPresetsByCategory (cat);
    presetList.updateContent();
    presetList.repaint();

    // Keep toggle states in sync
    for (int i = 0; i < (int) catBtns.size(); ++i)
        catBtns[(size_t)i]->setToggleState (kCategories[i] == cat,
                                             juce::dontSendNotification);
}

// ============================================================
// Layout
// ============================================================
void BrowserPanel::resized ()
{
    const int pw = juce::jmin (880, getWidth()  - 60);
    const int ph = juce::jmin (520, getHeight() - 80);
    panel = juce::Rectangle<int> ((getWidth()  - pw) / 2,
                                  (getHeight() - ph) / 2,
                                  pw, ph);

    // Header row inside panel
    const int headerH = 42;
    auto hdr = panel.withHeight (headerH);
    closeBtn.setBounds (hdr.getRight() - 44, hdr.getY() + 8, 32, 26);

    // Below header
    auto body = panel.withTrimmedTop (headerH).reduced (10, 8);

    // Left sidebar: 160px
    sidebar  = body.removeFromLeft (160);
    body.removeFromLeft (8); // gap
    listArea = body;

    // Category buttons — stacked in sidebar
    const int btnH = 32, gap = 5;
    auto sideWork = sidebar;
    for (auto& btn : catBtns)
    {
        btn->setBounds (sideWork.removeFromTop (btnH));
        sideWork.removeFromTop (gap);
    }

    presetList.setBounds (listArea);
}

// ============================================================
// Paint
// ============================================================
void BrowserPanel::paint (juce::Graphics& g)
{
    // Semi-transparent dark scrim over the whole editor
    g.setColour (juce::Colour (0xd0060810));
    g.fillAll();

    // Panel card
    g.setColour (MSColors::panelBg);
    g.fillRoundedRectangle (panel.toFloat(), 10.0f);
    g.setColour (MSColors::border);
    g.drawRoundedRectangle (panel.toFloat(), 10.0f, 1.5f);

    // Header background
    const int headerH = 42;
    auto hdrR = panel.withHeight (headerH).toFloat();
    juce::Path hdrPath;
    hdrPath.addRoundedRectangle (hdrR.getX(), hdrR.getY(),
                                 hdrR.getWidth(), hdrR.getHeight(),
                                 10.0f, 10.0f, true, true, false, false);
    g.setColour (MSColors::panelAlt);
    g.fillPath (hdrPath);

    // Header title
    g.setColour (MSColors::accentHi);
    g.setFont (juce::Font (15.0f, juce::Font::bold));
    g.drawText ("PRESET BROWSER",
                panel.getX() + 16, panel.getY() + 10,
                300, 22, juce::Justification::centredLeft);

    // Sidebar label
    g.setColour (MSColors::textDim);
    g.setFont (juce::Font (8.5f, juce::Font::bold));
    g.drawText ("CATEGORY",
                sidebar.getX(), sidebar.getY() - 14,
                sidebar.getWidth(), 12, juce::Justification::centredLeft);

    // List label
    g.drawText ("PRESETS  (" + juce::String (filteredPresets.size()) + ")",
                listArea.getX(), listArea.getY() - 14,
                listArea.getWidth(), 12, juce::Justification::centredLeft);
}

// ============================================================
// ListBoxModel
// ============================================================
void BrowserPanel::paintListBoxItem (int row, juce::Graphics& g,
                                     int w, int h, bool selected)
{
    if (selected)
    {
        g.setColour (MSColors::accent.withAlpha (0.25f));
        g.fillRect (0, 0, w, h);
        g.setColour (MSColors::accent);
        g.fillRect (0, 0, 3, h);
    }
    else if (row % 2 == 0)
    {
        g.setColour (juce::Colour (0xff0f1120));
        g.fillRect (0, 0, w, h);
    }

    const juce::String name = filteredPresets[row];
    const juce::String cat  = presets.getCategory (name);

    // Preset name
    g.setColour (selected ? juce::Colours::white : MSColors::textHi);
    g.setFont (juce::Font (12.5f));
    g.drawText (name, 12, 0, w - 90, h, juce::Justification::centredLeft);

    // Category tag badge
    if (cat.isNotEmpty() && cat != "ALL")
    {
        const int tagW = 64;
        const int tagX = w - tagW - 8;
        const int tagH = 14;
        const int tagY = (h - tagH) / 2;

        g.setColour (MSColors::accentDim.withAlpha (0.5f));
        g.fillRoundedRectangle ((float)tagX, (float)tagY,
                                (float)tagW, (float)tagH, 4.0f);
        g.setColour (MSColors::accentHi);
        g.setFont (juce::Font (8.5f, juce::Font::bold));
        g.drawText (cat.toUpperCase(), tagX, tagY, tagW, tagH,
                    juce::Justification::centred);
    }

    // Divider
    g.setColour (MSColors::border.withAlpha (0.4f));
    g.drawHorizontalLine (h - 1, 8.0f, (float)(w - 8));
}

void BrowserPanel::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    if (row >= 0 && row < filteredPresets.size())
    {
        presets.loadPreset (filteredPresets[row]);
        if (onPresetLoaded) onPresetLoaded();
    }
}

void BrowserPanel::listBoxItemDoubleClicked (int row, const juce::MouseEvent& e)
{
    listBoxItemClicked (row, e);
    if (onClose) onClose(); // double-click also closes the browser
}

juce::String BrowserPanel::getTooltipForRow (int row)
{
    if (row >= 0 && row < filteredPresets.size())
        return "Load: " + filteredPresets[row];
    return {};
}
