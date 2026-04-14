#include "LicenseOverlay.h"

LicenseOverlay::LicenseOverlay (LicenseManager& lm)
    : licenseManager (lm)
{
    // Title
    titleLabel.setText ("DUCKTANE", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (42.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, MSColors::accentHi);
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    // Subtitle
    subtitleLabel.setText ("Enter your license key to activate", juce::dontSendNotification);
    subtitleLabel.setFont (juce::Font (15.0f));
    subtitleLabel.setColour (juce::Label::textColourId, MSColors::textMid);
    subtitleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (subtitleLabel);

    // "LICENSE KEY" label above the text field
    keyLabel.setText ("LICENSE KEY", juce::dontSendNotification);
    keyLabel.setFont (juce::Font (10.0f, juce::Font::bold));
    keyLabel.setColour (juce::Label::textColourId, MSColors::textMid);
    keyLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (keyLabel);

    // Key input field
    keyField.setMultiLine (false);
    keyField.setReturnKeyStartsNewLine (false);
    keyField.setTextToShowWhenEmpty ("DUCK-XXXX-XXXX-XXXX", MSColors::textDim);
    keyField.setFont (juce::Font (juce::Font::getDefaultMonospacedFontName(), 16.0f, juce::Font::plain));
    keyField.setColour (juce::TextEditor::backgroundColourId,   MSColors::panelAlt);
    keyField.setColour (juce::TextEditor::outlineColourId,      MSColors::border);
    keyField.setColour (juce::TextEditor::focusedOutlineColourId, MSColors::accent);
    keyField.setColour (juce::TextEditor::textColourId,         MSColors::textHi);
    keyField.setColour (juce::CaretComponent::caretColourId,    MSColors::accentHi);
    keyField.setInputRestrictions (19); // "DUCK-XXXX-XXXX-XXXX" = 19 chars
    keyField.onReturnKey = [this] { tryActivate(); };
    addAndMakeVisible (keyField);

    // Activate button
    activateBtn.onClick = [this] { tryActivate(); };
    addAndMakeVisible (activateBtn);

    // Status message
    statusLabel.setFont (juce::Font (12.0f));
    statusLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (statusLabel);
}

void LicenseOverlay::paint (juce::Graphics& g)
{
    // Semi-opaque dark backdrop
    g.fillAll (MSColors::bg.withAlpha (0.97f));

    // Card background
    const auto card = getLocalBounds().withSizeKeepingCentre (480, 340).toFloat();
    g.setColour (MSColors::panelBg);
    g.fillRoundedRectangle (card, 12.0f);
    g.setColour (MSColors::border);
    g.drawRoundedRectangle (card, 12.0f, 1.5f);

    // Accent top stripe on card
    const auto stripe = card.withHeight (4.0f).withY (card.getY());
    g.setColour (MSColors::accent);
    g.fillRoundedRectangle (stripe, 3.0f);
}

void LicenseOverlay::resized()
{
    const auto card = getLocalBounds().withSizeKeepingCentre (480, 340);
    auto area = card.reduced (40, 32);

    titleLabel   .setBounds (area.removeFromTop (52));
    subtitleLabel.setBounds (area.removeFromTop (24));
    area.removeFromTop (24); // spacing

    keyLabel     .setBounds (area.removeFromTop (16));
    keyField     .setBounds (area.removeFromTop (38));
    area.removeFromTop (14);

    activateBtn  .setBounds (area.removeFromTop (38).withSizeKeepingCentre (160, 38));
    area.removeFromTop (10);

    statusLabel  .setBounds (area.removeFromTop (22));
}

// ---- Private ----------------------------------------------------------------

void LicenseOverlay::tryActivate()
{
    const juce::String raw = keyField.getText();
    const juce::String key = LicenseManager::normalise (raw);

    if (key.isEmpty())
    {
        updateStatus ("Please enter a license key.", true);
        return;
    }

    if (! LicenseManager::isKeyValid (key))
    {
        updateStatus ("Invalid key. Format: DUCK-XXXX-XXXX-XXXX", true);
        return;
    }

    if (! licenseManager.activate (key))
    {
        updateStatus ("Could not save license. Check disk permissions.", true);
        return;
    }

    updateStatus ("Activated successfully!", false);

    juce::Timer::callAfterDelay (800, [this]
    {
        if (onActivated) onActivated();
    });
}

void LicenseOverlay::updateStatus (const juce::String& msg, bool isError)
{
    statusLabel.setText (msg, juce::dontSendNotification);
    statusLabel.setColour (juce::Label::textColourId,
                           isError ? juce::Colour (0xffe05252) : MSColors::green);
    repaint();
}
