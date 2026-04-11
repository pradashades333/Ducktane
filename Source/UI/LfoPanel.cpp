#include "LfoPanel.h"

LfoPanel::LfoPanel (juce::AudioProcessorValueTreeState& apvts,
                    const juce::String& prefix,
                    const juce::String& ttl)
    : title (ttl)
{
    addAndMakeVisible (shapeBox);
    shapeBox.addItemList ({ "Sine", "Triangle", "Saw", "Square", "S&H" }, 1);

    addAndMakeVisible (targetBox);
    targetBox.addItemList ({ "None", "Filter Cutoff", "OSC 1+2 Pitch",
                             "OSC 1 Pitch", "OSC 2 Pitch" }, 1);

    MSLayout::setupKnob (*this, rateKnob,  rateLbl,  "RATE");
    MSLayout::setupKnob (*this, depthKnob, depthLbl, "DEPTH");

    shapeAtt  = std::make_unique<CbAtt> (apvts, prefix + "shape",  shapeBox);
    targetAtt = std::make_unique<CbAtt> (apvts, prefix + "target", targetBox);
    rateAtt   = std::make_unique<SlAtt> (apvts, prefix + "rate",   rateKnob);
    depthAtt  = std::make_unique<SlAtt> (apvts, prefix + "depth",  depthKnob);
}

void LfoPanel::paint (juce::Graphics& g)
{
    MSLayout::paintSection (g, getLocalBounds(), title, MSColors::accentDim.brighter (0.15f));
}

void LfoPanel::resized()
{
    auto area = getLocalBounds().reduced (6);
    area.removeFromTop (22); // title

    // Top row: shape combo (left) + target combo (right)
    {
        auto row = area.removeFromTop (22);
        shapeBox.setBounds  (row.removeFromLeft (row.getWidth() / 2).reduced (2, 0));
        targetBox.setBounds (row.reduced (2, 0));
    }
    area.removeFromTop (4);

    // Knob row: Rate + Depth
    {
        const int w = area.getWidth() / 2;
        MSLayout::placeKnob (rateKnob,  rateLbl,  area.removeFromLeft (w));
        MSLayout::placeKnob (depthKnob, depthLbl, area);
    }
}
