#include "OscillatorPanel.h"

OscillatorPanel::OscillatorPanel (juce::AudioProcessorValueTreeState& apvts,
                                   const juce::String& prefix,
                                   const juce::String& ttl)
    : title (ttl)
{
    addAndMakeVisible (waveDisplay);

    // Shape combo
    addAndMakeVisible (shapeBox);
    shapeBox.addItemList ({ "Sine", "Triangle", "Saw", "Square" }, 1);
    shapeBox.setColour (juce::ComboBox::textColourId, MSColors::textHi);

    // Knobs
    MSLayout::setupKnob (*this, octaveKnob, octaveLbl, "OCT");
    MSLayout::setupKnob (*this, semiKnob,   semiLbl,   "SEMI");
    MSLayout::setupKnob (*this, fineKnob,   fineLbl,   "FINE");
    MSLayout::setupKnob (*this, levelKnob,  levelLbl,  "LEVEL");
    MSLayout::setupKnob (*this, voicesKnob, voicesLbl, "VOICES");
    MSLayout::setupKnob (*this, detuneKnob, detuneLbl, "DETUNE");
    MSLayout::setupKnob (*this, spreadKnob, spreadLbl, "SPREAD");

    // Attachments (constructed after controls are set up)
    shapeAtt  = std::make_unique<CbAtt> (apvts, prefix + "shape",         shapeBox);
    octaveAtt = std::make_unique<SlAtt> (apvts, prefix + "octave",        octaveKnob);
    semiAtt   = std::make_unique<SlAtt> (apvts, prefix + "semi",          semiKnob);
    fineAtt   = std::make_unique<SlAtt> (apvts, prefix + "fine",          fineKnob);
    levelAtt  = std::make_unique<SlAtt> (apvts, prefix + "level",         levelKnob);
    voicesAtt = std::make_unique<SlAtt> (apvts, prefix + "unison_voices", voicesKnob);
    detuneAtt = std::make_unique<SlAtt> (apvts, prefix + "unison_detune", detuneKnob);
    spreadAtt = std::make_unique<SlAtt> (apvts, prefix + "unison_spread", spreadKnob);
}

void OscillatorPanel::paint (juce::Graphics& g)
{
    MSLayout::paintSection (g, getLocalBounds(), title);

    // Unison sub-header
    g.setColour (MSColors::border);
    g.drawHorizontalLine (unisonDivY, 8.0f, static_cast<float> (getWidth()) - 8.0f);
    g.setColour (MSColors::textDim);
    g.setFont (juce::Font (8.5f, juce::Font::bold));
    g.drawText ("UNISON", 0, unisonDivY - 9, getWidth(), 10, juce::Justification::centred);
}

void OscillatorPanel::resized()
{
    auto area = getLocalBounds().reduced (8);
    area.removeFromTop (22); // title bar

    // Shape combo
    shapeBox.setBounds (area.removeFromTop (22));
    area.removeFromTop (6);

    // Waveform display — takes a meaningful chunk so it's easy to read
    waveDisplay.setBounds (area.removeFromTop (110));
    area.removeFromTop (6);

    // Tune row: Oct | Semi | Fine
    {
        auto row = area.removeFromTop (72);
        const int w = row.getWidth() / 3;
        MSLayout::placeKnob (octaveKnob, octaveLbl, row.removeFromLeft (w));
        MSLayout::placeKnob (semiKnob,   semiLbl,   row.removeFromLeft (w));
        MSLayout::placeKnob (fineKnob,   fineLbl,   row);
    }
    area.removeFromTop (6);

    // Level row (centered, half width)
    {
        auto row  = area.removeFromTop (72);
        const int hw = row.getWidth() / 2;
        const int cx = row.getX() + hw / 2;
        MSLayout::placeKnob (levelKnob, levelLbl,
                              juce::Rectangle<int> (cx, row.getY(), hw, row.getHeight()));
    }
    area.removeFromTop (10);

    // Unison divider bookmark
    unisonDivY = area.getY() - getLocalBounds().getY() + 2;
    area.removeFromTop (10);

    // Unison row: Voices | Detune | Spread
    {
        auto row = area.removeFromTop (72);
        const int w = row.getWidth() / 3;
        MSLayout::placeKnob (voicesKnob, voicesLbl, row.removeFromLeft (w));
        MSLayout::placeKnob (detuneKnob, detuneLbl, row.removeFromLeft (w));
        MSLayout::placeKnob (spreadKnob, spreadLbl, row);
    }
}
