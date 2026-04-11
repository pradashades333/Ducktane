#include "FilterPanel.h"

FilterPanel::FilterPanel (juce::AudioProcessorValueTreeState& apvts)
{
    logoImage = juce::ImageCache::getFromMemory (
        DucktaneAssets::logo_png, DucktaneAssets::logo_pngSize);
    addAndMakeVisible (filterTypeBox);
    filterTypeBox.addItemList ({ "LP 12dB", "LP 24dB", "HP", "BP" }, 1);

    MSLayout::setupKnob (*this, cutoffKnob, cutoffLbl, "CUTOFF");
    MSLayout::setupKnob (*this, resKnob,    resLbl,    "RESO");
    MSLayout::setupKnob (*this, driveKnob,  driveLbl,  "DRIVE");
    MSLayout::setupKnob (*this, envAmtKnob, envAmtLbl, "ENV AMT");

    MSLayout::setupKnob (*this, subLevelKnob, subLevelLbl, "LEVEL");

    addAndMakeVisible (subOctaveBox);
    subOctaveBox.addItemList ({ "-2 Oct", "-1 Oct" }, 1);

    filterTypeAtt = std::make_unique<CbAtt> (apvts, "filter_type",    filterTypeBox);
    cutoffAtt     = std::make_unique<SlAtt> (apvts, "filter_cutoff",  cutoffKnob);
    resAtt        = std::make_unique<SlAtt> (apvts, "filter_res",     resKnob);
    driveAtt      = std::make_unique<SlAtt> (apvts, "filter_drive",   driveKnob);
    envAmtAtt     = std::make_unique<SlAtt> (apvts, "filter_env_amt", envAmtKnob);
    subLevelAtt   = std::make_unique<SlAtt> (apvts, "sub_level",      subLevelKnob);
    subOctaveAtt  = std::make_unique<CbAtt> (apvts, "sub_octave",     subOctaveBox);
}

void FilterPanel::paint (juce::Graphics& g)
{
    MSLayout::paintSection (g, getLocalBounds(), "FILTER");

    // Sub divider
    g.setColour (MSColors::border);
    g.drawHorizontalLine (subDivY, 8.0f, static_cast<float> (getWidth()) - 8.0f);
    g.setColour (MSColors::textDim);
    g.setFont (juce::Font (8.5f, juce::Font::bold));
    g.drawText ("SUB OSC", 0, subDivY - 9, getWidth(), 10, juce::Justification::centred);

    // Logo in remaining bottom space
    if (logoImage.isValid() && ! logoArea.isEmpty())
    {
        const float aspect = (float) logoImage.getWidth() / (float) logoImage.getHeight();
        const int   drawW  = juce::jmin (logoArea.getWidth() - 20,
                                         (int) ((logoArea.getHeight() - 16) * aspect));
        const int   drawH  = (int) (drawW / aspect);
        const int   drawX  = logoArea.getX() + (logoArea.getWidth()  - drawW) / 2;
        const int   drawY  = logoArea.getY() + (logoArea.getHeight() - drawH) / 2;

        g.setOpacity (0.55f);
        g.drawImage (logoImage, drawX, drawY, drawW, drawH,
                     0, 0, logoImage.getWidth(), logoImage.getHeight());
        g.setOpacity (1.0f);
    }
}

void FilterPanel::resized()
{
    auto area = getLocalBounds().reduced (8);
    area.removeFromTop (22);

    filterTypeBox.setBounds (area.removeFromTop (22));
    area.removeFromTop (6);

    // Filter knobs: 2×2 grid
    {
        const int half = area.getWidth() / 2;
        auto row1 = area.removeFromTop (72);
        MSLayout::placeKnob (cutoffKnob, cutoffLbl, row1.removeFromLeft (half));
        MSLayout::placeKnob (resKnob,    resLbl,    row1);
        area.removeFromTop (4);
        auto row2 = area.removeFromTop (72);
        MSLayout::placeKnob (driveKnob,  driveLbl,  row2.removeFromLeft (half));
        MSLayout::placeKnob (envAmtKnob, envAmtLbl, row2);
    }
    area.removeFromTop (10);

    subDivY = area.getY() - getLocalBounds().getY() + 2;
    area.removeFromTop (10);

    // Sub level + octave
    {
        const int half = area.getWidth() / 2;
        auto row = area.removeFromTop (72);
        MSLayout::placeKnob (subLevelKnob, subLevelLbl, row.removeFromLeft (half));
        // Octave combo centred in right half
        auto right = row;
        subOctaveBox.setBounds (right.withSizeKeepingCentre (right.getWidth() - 8, 22));
    }
    area.removeFromTop (8);

    // Remaining space → logo
    logoArea = area;
}
