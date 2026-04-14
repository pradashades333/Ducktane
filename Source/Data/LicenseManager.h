#pragma once
#include <JuceHeader.h>

/**
 * Manages Ducktane license key validation and persistent storage.
 *
 * Key format:  DUCK-AAAA-BBBB-CCCC
 *   where AAAA, BBBB, CCCC are 4 uppercase hex digits each.
 *   A key is valid when:  0xAAAA XOR 0xBBBB == 0xCCCC
 *
 * To generate a valid key:
 *   1. Pick any 4-hex-digit value for A  (e.g. 4275)
 *   2. Pick any 4-hex-digit value for B  (e.g. 1337)
 *   3. C = A XOR B  ->  DUCK-4275-1337-5142
 *
 * The validated key is stored (hashed) in:
 *   Windows:  %APPDATA%/Ducktane/license.lic
 *   macOS:    ~/Library/Application Support/Ducktane/license.lic
 */
class LicenseManager
{
public:
    LicenseManager();

    /** Returns true if a valid license key has already been saved. */
    bool isActivated() const;

    /**
     * Validates the supplied key string and, if correct, persists it to disk.
     * Returns true on success.
     */
    bool activate (const juce::String& rawKey);

    /** Normalises the key (trims whitespace, converts to uppercase). */
    static juce::String normalise (const juce::String& raw);

    /** Pure validation — does NOT store the key. */
    static bool isKeyValid (const juce::String& normalisedKey);

private:
    juce::File licenseFile;
    bool       activated { false };

    void checkActivation();

    /** Returns a simple one-way hash string used to verify the stored key. */
    static juce::String hashKey (const juce::String& normalisedKey);
};
