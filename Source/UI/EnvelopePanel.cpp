#include "EnvelopePanel.h"

EnvelopePanel::EnvelopePanel (juce::AudioProcessorValueTreeState& apvts,
                               const juce::String& prefix,
                               const juce::String& ttl)
    : title (ttl)
{
    MSLayout::setupKnob (*this, attackKnob,  attackLbl,  "ATK");
    MSLayout::setupKnob (*this, decayKnob,   decayLbl,   "DEC");
    MSLayout::setupKnob (*this, sustainKnob, sustainLbl, "SUS");
    MSLayout::setupKnob (*this, releaseKnob, releaseLbl, "REL");

    attackAtt  = std::make_unique<SlAtt> (apvts, prefix + "attack",  attackKnob);
    decayAtt   = std::make_unique<SlAtt> (apvts, prefix + "decay",   decayKnob);
    sustainAtt = std::make_unique<SlAtt> (apvts, prefix + "sustain", sustainKnob);
    releaseAtt = std::make_unique<SlAtt> (apvts, prefix + "release", releaseKnob);
}

void EnvelopePanel::paint (juce::Graphics& g)
{
    MSLayout::paintSection (g, getLocalBounds(), title, MSColors::accentDim.brighter (0.3f));
}

void EnvelopePanel::resized()
{
    auto area = getLocalBounds().reduced (6);
    area.removeFromTop (22); // title

    // 4 knobs in a row
    const int w = area.getWidth() / 4;
    MSLayout::placeKnob (attackKnob,  attackLbl,  area.removeFromLeft (w));
    MSLayout::placeKnob (decayKnob,   decayLbl,   area.removeFromLeft (w));
    MSLayout::placeKnob (sustainKnob, sustainLbl, area.removeFromLeft (w));
    MSLayout::placeKnob (releaseKnob, releaseLbl, area);
}
