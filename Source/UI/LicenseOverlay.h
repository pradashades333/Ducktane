#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include "../Data/LicenseManager.h"

/**
 * Full-screen overlay that asks the user to enter a license key.
 * Shown on startup when no valid license is found.
 * Hidden permanently once a valid key is entered and saved.
 *
 * Usage:
 *   addChildComponent (licenseOverlay);
 *   licenseOverlay.setBounds (getLocalBounds());
 *   if (! licenseManager.isActivated())
 *       licenseOverlay.setVisible (true);
 *
 *   licenseOverlay.onActivated = [this] { licenseOverlay.setVisible (false); };
 */
class LicenseOverlay : public juce::Component
{
public:
    explicit LicenseOverlay (LicenseManager& lm);

    void paint   (juce::Graphics&) override;
    void resized () override;

    /** Called after a successful activation. Wire this to hide the overlay. */
    std::function<void()> onActivated;

private:
    LicenseManager& licenseManager;

    juce::Label     titleLabel;
    juce::Label     subtitleLabel;
    juce::Label     keyLabel;
    juce::TextEditor keyField;
    juce::TextButton activateBtn { "ACTIVATE" };
    juce::Label     statusLabel;

    void tryActivate();
    void updateStatus (const juce::String& msg, bool isError);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LicenseOverlay)
};
