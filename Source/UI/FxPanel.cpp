#include "FxPanel.h"

FxPanel::FxPanel (juce::AudioProcessorValueTreeState& apvts)
{
    auto sk = [&] (juce::Slider& s, juce::Label& l, const juce::String& txt)
    {
        MSLayout::setupKnob (*this, s, l, txt);
    };

    sk (distDriveKnob, distDriveLbl, "DRIVE");
    sk (distMixKnob,   distMixLbl,   "MIX");

    sk (phaRateKnob,   phaRateLbl,   "RATE");
    sk (phaDepthKnob,  phaDepthLbl,  "DEPTH");
    sk (phaMixKnob,    phaMixLbl,    "MIX");

    sk (chorRateKnob,  chorRateLbl,  "RATE");
    sk (chorDepthKnob, chorDepthLbl, "DEPTH");
    sk (chorMixKnob,   chorMixLbl,   "MIX");

    sk (delayTimeKnob, delayTimeLbl, "TIME");
    sk (delayFbKnob,   delayFbLbl,   "FDBK");
    sk (delayMixKnob,  delayMixLbl,  "MIX");

    sk (revRoomKnob,   revRoomLbl,   "ROOM");
    sk (revDampKnob,   revDampLbl,   "DAMP");
    sk (revMixKnob,    revMixLbl,    "MIX");

    sk (crushBitsKnob, crushBitsLbl, "BITS");
    sk (crushMixKnob,  crushMixLbl,  "MIX");

    distDriveAtt  = std::make_unique<SlAtt> (apvts, "dist_drive",      distDriveKnob);
    distMixAtt    = std::make_unique<SlAtt> (apvts, "dist_mix",        distMixKnob);
    phaRateAtt    = std::make_unique<SlAtt> (apvts, "phaser_rate",     phaRateKnob);
    phaDepthAtt   = std::make_unique<SlAtt> (apvts, "phaser_depth",    phaDepthKnob);
    phaMixAtt     = std::make_unique<SlAtt> (apvts, "phaser_mix",      phaMixKnob);
    chorRateAtt   = std::make_unique<SlAtt> (apvts, "chorus_rate",     chorRateKnob);
    chorDepthAtt  = std::make_unique<SlAtt> (apvts, "chorus_depth",    chorDepthKnob);
    chorMixAtt    = std::make_unique<SlAtt> (apvts, "chorus_mix",      chorMixKnob);
    delayTimeAtt  = std::make_unique<SlAtt> (apvts, "delay_time",      delayTimeKnob);
    delayFbAtt    = std::make_unique<SlAtt> (apvts, "delay_feedback",  delayFbKnob);
    delayMixAtt   = std::make_unique<SlAtt> (apvts, "delay_mix",       delayMixKnob);
    revRoomAtt    = std::make_unique<SlAtt> (apvts, "reverb_room",     revRoomKnob);
    revDampAtt    = std::make_unique<SlAtt> (apvts, "reverb_damp",     revDampKnob);
    revMixAtt     = std::make_unique<SlAtt> (apvts, "reverb_mix",      revMixKnob);
    crushBitsAtt  = std::make_unique<SlAtt> (apvts, "crush_bits",      crushBitsKnob);
    crushMixAtt   = std::make_unique<SlAtt> (apvts, "crush_mix",       crushMixKnob);
}

void FxPanel::drawFxSection (juce::Graphics& g, juce::Rectangle<int> bounds,
                               const juce::String& name) const
{
    g.setColour (MSColors::panelAlt);
    g.fillRoundedRectangle (bounds.toFloat().reduced (2.0f), 6.0f);
    g.setColour (MSColors::border);
    g.drawRoundedRectangle (bounds.toFloat().reduced (2.0f), 6.0f, 1.0f);
    g.setColour (MSColors::textDim);
    g.setFont (juce::Font (9.0f, juce::Font::bold));
    g.drawText (name, bounds.getX(), bounds.getY() + 6,
                bounds.getWidth(), 12, juce::Justification::centred);
}

void FxPanel::paint (juce::Graphics& g)
{
    g.setColour (MSColors::bg);
    g.fillRect (getLocalBounds());

    const int w = getWidth() / 6;
    drawFxSection (g, { w * 0, 0, w,                    getHeight() }, "DISTORTION");
    drawFxSection (g, { w * 1, 0, w,                    getHeight() }, "PHASER");
    drawFxSection (g, { w * 2, 0, w,                    getHeight() }, "CHORUS");
    drawFxSection (g, { w * 3, 0, w,                    getHeight() }, "DELAY");
    drawFxSection (g, { w * 4, 0, w,                    getHeight() }, "REVERB");
    drawFxSection (g, { w * 5, 0, getWidth() - w * 5,  getHeight() }, "BITCRUSHER");
}

void FxPanel::resized()
{
    const int w   = getWidth() / 6;
    const int top = 24;
    const int kH  = getHeight() - top - 4;

    auto place3 = [&] (juce::Rectangle<int> sec,
                        juce::Slider& k1, juce::Label& l1,
                        juce::Slider& k2, juce::Label& l2,
                        juce::Slider& k3, juce::Label& l3)
    {
        sec = sec.reduced (4, 0).withTop (top);
        const int kw = sec.getWidth() / 3;
        MSLayout::placeKnob (k1, l1, sec.removeFromLeft (kw).withHeight (kH));
        MSLayout::placeKnob (k2, l2, sec.removeFromLeft (kw).withHeight (kH));
        MSLayout::placeKnob (k3, l3, sec.withHeight (kH));
    };

    auto place2 = [&] (juce::Rectangle<int> sec,
                        juce::Slider& k1, juce::Label& l1,
                        juce::Slider& k2, juce::Label& l2)
    {
        sec = sec.reduced (4, 0).withTop (top);
        const int kw = sec.getWidth() / 2;
        MSLayout::placeKnob (k1, l1, sec.removeFromLeft (kw).withHeight (kH));
        MSLayout::placeKnob (k2, l2, sec.withHeight (kH));
    };

    place2 ({ w * 0, 0, w,                   getHeight() }, distDriveKnob, distDriveLbl, distMixKnob,  distMixLbl);
    place3 ({ w * 1, 0, w,                   getHeight() }, phaRateKnob,   phaRateLbl,   phaDepthKnob,  phaDepthLbl,  phaMixKnob,   phaMixLbl);
    place3 ({ w * 2, 0, w,                   getHeight() }, chorRateKnob,  chorRateLbl,  chorDepthKnob, chorDepthLbl, chorMixKnob,  chorMixLbl);
    place3 ({ w * 3, 0, w,                   getHeight() }, delayTimeKnob, delayTimeLbl, delayFbKnob,   delayFbLbl,   delayMixKnob, delayMixLbl);
    place3 ({ w * 4, 0, w,                   getHeight() }, revRoomKnob,   revRoomLbl,   revDampKnob,   revDampLbl,   revMixKnob,   revMixLbl);
    place2 ({ w * 5, 0, getWidth() - w * 5,  getHeight() }, crushBitsKnob, crushBitsLbl, crushMixKnob,  crushMixLbl);
}
