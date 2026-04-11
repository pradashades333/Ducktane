#pragma once
#include <JuceHeader.h>

// ============================================================
// Shared color palette — used by all panels
// ============================================================
namespace MSColors
{
    static const juce::Colour bg       { 0xff0d0f1a };
    static const juce::Colour panelBg  { 0xff161929 };
    static const juce::Colour panelAlt { 0xff1c1f33 };
    static const juce::Colour border   { 0xff272b45 };
    static const juce::Colour accent   { 0xff7c3aed };
    static const juce::Colour accentHi { 0xff9d5cf6 };
    static const juce::Colour accentDim{ 0xff3b1d70 };
    static const juce::Colour knobBody { 0xff20233a };
    static const juce::Colour knobRim  { 0xff131523 };
    static const juce::Colour textHi   { 0xffe2e8f0 };
    static const juce::Colour textMid  { 0xff7888a8 };
    static const juce::Colour textDim  { 0xff3d4a62 };
    static const juce::Colour green    { 0xff22c55e };
}

// ============================================================
// CustomLookAndFeel
// ============================================================
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox&) override;

    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;

    void drawPopupMenuBackground (juce::Graphics&, int width, int height) override;

    void drawPopupMenuItem (juce::Graphics&, const juce::Rectangle<int>&,
                            bool isSeparator, bool isActive, bool isHighlighted,
                            bool isTicked, bool hasSubMenu, const juce::String& text,
                            const juce::String& shortcutKeyText,
                            const juce::Drawable* icon,
                            const juce::Colour* textColour) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
};

// ============================================================
// Shared helpers used by all panels
// ============================================================
namespace MSLayout
{
    /** Set up a rotary knob with a small text box below. */
    inline void setupKnob (juce::Component& parent, juce::Slider& s, juce::Label& l,
                           const juce::String& labelText)
    {
        parent.addAndMakeVisible (s);
        s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 58, 13);
        s.setColour (juce::Slider::textBoxTextColourId,    MSColors::textMid);
        s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        s.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);

        parent.addAndMakeVisible (l);
        l.setText (labelText, juce::dontSendNotification);
        l.setFont (juce::Font (9.0f, juce::Font::bold));
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, MSColors::textMid);
    }

    /** Place a knob+label pair inside `area` — label is 13px at the bottom. */
    inline void placeKnob (juce::Slider& s, juce::Label& l, juce::Rectangle<int> area)
    {
        l.setBounds (area.removeFromBottom (13));
        s.setBounds (area);
    }

    /** Draw a panel background + title bar. */
    inline void paintSection (juce::Graphics& g, juce::Rectangle<int> bounds,
                               const juce::String& title, juce::Colour titleColour = MSColors::accent)
    {
        g.setColour (MSColors::panelBg);
        g.fillRoundedRectangle (bounds.toFloat().reduced (1.5f), 7.0f);
        g.setColour (MSColors::border);
        g.drawRoundedRectangle (bounds.toFloat().reduced (1.5f), 7.0f, 1.0f);

        juce::Rectangle<int> titleBar (bounds.getX() + 8, bounds.getY() + 6,
                                       bounds.getWidth() - 16, 16);
        g.setColour (titleColour);
        g.fillRoundedRectangle (titleBar.toFloat(), 3.0f);
        g.setColour (MSColors::textHi);
        g.setFont (juce::Font (9.5f, juce::Font::bold));
        g.drawText (title, titleBar, juce::Justification::centred);
    }
}
