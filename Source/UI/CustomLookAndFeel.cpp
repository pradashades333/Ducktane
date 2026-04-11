#include "CustomLookAndFeel.h"

CustomLookAndFeel::CustomLookAndFeel()
{
    // Global colour overrides
    setColour (juce::ResizableWindow::backgroundColourId, MSColors::bg);
    setColour (juce::PopupMenu::backgroundColourId,       MSColors::panelAlt);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, MSColors::accentDim);
    setColour (juce::PopupMenu::textColourId,             MSColors::textHi);
    setColour (juce::PopupMenu::highlightedTextColourId,  MSColors::textHi);
    setColour (juce::ComboBox::backgroundColourId,        MSColors::panelAlt);
    setColour (juce::ComboBox::outlineColourId,           MSColors::border);
    setColour (juce::ComboBox::textColourId,              MSColors::textHi);
    setColour (juce::ComboBox::arrowColourId,             MSColors::textMid);
    setColour (juce::TextButton::buttonColourId,          MSColors::accentDim);
    setColour (juce::TextButton::textColourOffId,         MSColors::textHi);
}

// ============================================================
// Rotary knob
// ============================================================
void CustomLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                           int x, int y, int width, int height,
                                           float sliderPos,
                                           float rotaryStartAngle, float rotaryEndAngle,
                                           juce::Slider& /*slider*/)
{
    const float cx = static_cast<float> (x) + static_cast<float> (width)  * 0.5f;
    const float cy = static_cast<float> (y) + static_cast<float> (height) * 0.5f;
    const float r  = static_cast<float> (juce::jmin (width, height)) * 0.5f - 5.0f;

    if (r < 3.0f) return;

    // ---- Shadow ----
    {
        juce::Path shadow;
        shadow.addEllipse (cx - r - 1, cy - r + 1, (r + 1) * 2.0f, (r + 1) * 2.0f);
        juce::DropShadow (juce::Colours::black.withAlpha (0.6f), 8, { 0, 3 }).drawForPath (g, shadow);
    }

    // ---- Body gradient ----
    {
        juce::ColourGradient grad (MSColors::knobBody.brighter (0.1f), cx, cy - r,
                                   MSColors::knobRim, cx, cy + r * 0.8f, false);
        g.setGradientFill (grad);
        g.fillEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f);
    }

    // ---- Rim ----
    g.setColour (MSColors::border);
    g.drawEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f, 1.5f);

    // ---- Track arc ----
    const float arcR = r - 5.0f;
    {
        juce::Path track;
        track.addArc (cx - arcR, cy - arcR, arcR * 2.0f, arcR * 2.0f,
                      rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (MSColors::textDim);
        g.strokePath (track, juce::PathStrokeType (2.5f,
                      juce::PathStrokeType::curved,
                      juce::PathStrokeType::rounded));
    }

    // ---- Value arc ----
    {
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        juce::Path arc;
        arc.addArc (cx - arcR, cy - arcR, arcR * 2.0f, arcR * 2.0f,
                    rotaryStartAngle, angle, true);
        g.setColour (MSColors::accent);
        g.strokePath (arc, juce::PathStrokeType (2.5f,
                      juce::PathStrokeType::curved,
                      juce::PathStrokeType::rounded));
    }

    // ---- Pointer line ----
    {
        const float angle  = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const float len    = r * 0.55f;
        const float px     = cx + len * std::sin (angle);
        const float py     = cy - len * std::cos (angle);
        g.setColour (juce::Colours::white.withAlpha (0.85f));
        g.drawLine (cx, cy, px, py, 2.0f);

        // Accent centre dot
        g.setColour (MSColors::accentHi);
        g.fillEllipse (cx - 2.5f, cy - 2.5f, 5.0f, 5.0f);
    }
}

// ============================================================
// ComboBox
// ============================================================
void CustomLookAndFeel::drawComboBox (juce::Graphics& g,
                                       int width, int height, bool /*isButtonDown*/,
                                       int /*buttonX*/, int /*buttonY*/,
                                       int /*buttonW*/, int /*buttonH*/,
                                       juce::ComboBox& /*box*/)
{
    g.setColour (MSColors::panelAlt);
    g.fillRoundedRectangle (1.0f, 1.0f, static_cast<float> (width) - 2.0f,
                             static_cast<float> (height) - 2.0f, 4.0f);
    g.setColour (MSColors::border);
    g.drawRoundedRectangle (1.0f, 1.0f, static_cast<float> (width) - 2.0f,
                              static_cast<float> (height) - 2.0f, 4.0f, 1.0f);

    // Arrow
    const float arrowX = static_cast<float> (width) - 16.0f;
    const float arrowY = static_cast<float> (height) * 0.5f;
    juce::Path arrow;
    arrow.addTriangle (arrowX, arrowY - 3.0f,
                       arrowX + 6.0f, arrowY - 3.0f,
                       arrowX + 3.0f, arrowY + 3.0f);
    g.setColour (MSColors::textMid);
    g.fillPath (arrow);
}

void CustomLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (4, 1, box.getWidth() - 24, box.getHeight() - 2);
    label.setFont (juce::Font (11.0f));
}

// ============================================================
// Popup menu
// ============================================================
void CustomLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    g.setColour (MSColors::panelAlt);
    g.fillRoundedRectangle (0.0f, 0.0f, static_cast<float> (width),
                             static_cast<float> (height), 5.0f);
    g.setColour (MSColors::border);
    g.drawRoundedRectangle (0.5f, 0.5f, static_cast<float> (width) - 1.0f,
                              static_cast<float> (height) - 1.0f, 5.0f, 1.0f);
}

void CustomLookAndFeel::drawPopupMenuItem (juce::Graphics& g,
                                            const juce::Rectangle<int>& area,
                                            bool /*isSeparator*/, bool /*isActive*/,
                                            bool isHighlighted, bool /*isTicked*/,
                                            bool /*hasSubMenu*/, const juce::String& text,
                                            const juce::String& /*shortcut*/,
                                            const juce::Drawable* /*icon*/,
                                            const juce::Colour* /*textColour*/)
{
    if (isHighlighted)
    {
        g.setColour (MSColors::accentDim);
        g.fillRoundedRectangle (area.toFloat().reduced (2.0f, 0.0f), 4.0f);
    }
    g.setColour (isHighlighted ? MSColors::textHi : MSColors::textMid);
    g.setFont (juce::Font (11.0f));
    g.drawText (text, area.reduced (8, 0), juce::Justification::centredLeft);
}

// ============================================================
// Button
// ============================================================
void CustomLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                               juce::Button& button,
                                               const juce::Colour& /*bgColour*/,
                                               bool isHighlighted, bool isDown)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    juce::Colour fill = MSColors::accentDim;
    if (isDown)       fill = MSColors::accent;
    else if (isHighlighted) fill = MSColors::accentDim.brighter (0.15f);

    g.setColour (fill);
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (MSColors::accent);
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
}

juce::Font CustomLookAndFeel::getTextButtonFont (juce::TextButton&, int /*height*/)
{
    return juce::Font (10.5f, juce::Font::bold);
}
