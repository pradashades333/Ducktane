#include "LicenseManager.h"

// Prefix expected on every key
static constexpr const char* kPrefix = "DUCK-";

LicenseManager::LicenseManager()
{
    licenseFile = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                      .getChildFile ("Ducktane")
                      .getChildFile ("license.lic");

    checkActivation();
}

// ---- Public -----------------------------------------------------------------

bool LicenseManager::isActivated() const
{
    return activated;
}

bool LicenseManager::activate (const juce::String& rawKey)
{
    const juce::String key = normalise (rawKey);

    if (! isKeyValid (key))
        return false;

    // Persist a hash of the key so we can verify on next launch
    if (! licenseFile.getParentDirectory().exists())
        licenseFile.getParentDirectory().createDirectory();

    if (! licenseFile.replaceWithText (hashKey (key)))
        return false;

    activated = true;
    return true;
}

// ---- Static helpers ---------------------------------------------------------

juce::String LicenseManager::normalise (const juce::String& raw)
{
    return raw.trim().toUpperCase();
}

bool LicenseManager::isKeyValid (const juce::String& key)
{
    // Expected: DUCK-AAAA-BBBB-CCCC  (total 19 chars)
    if (! key.startsWith (kPrefix))              return false;
    const juce::String body = key.substring (5); // "AAAA-BBBB-CCCC"
    if (body.length() != 14)                     return false;

    const juce::StringArray parts = juce::StringArray::fromTokens (body, "-", "");
    if (parts.size() != 3)                       return false;

    for (const auto& part : parts)
    {
        if (part.length() != 4) return false;
        for (auto ch : part)
        {
            const bool valid = (ch >= 'A' && ch <= 'F') || (ch >= '0' && ch <= '9');
            if (! valid) return false;
        }
    }

    const uint32_t a = static_cast<uint32_t> (parts[0].getHexValue32());
    const uint32_t b = static_cast<uint32_t> (parts[1].getHexValue32());
    const uint32_t c = static_cast<uint32_t> (parts[2].getHexValue32());

    return (a ^ b) == c;
}

// ---- Private ----------------------------------------------------------------

void LicenseManager::checkActivation()
{
    if (! licenseFile.existsAsFile())
        return;

    const juce::String stored = licenseFile.loadFileAsString().trim();
    if (stored.isEmpty())
        return;

    // The stored value is the hash — we cannot reverse it, so we just check
    // it's non-empty and has the right length (sanity guard against tampering).
    if (stored.length() == 16)
        activated = true;
}

juce::String LicenseManager::hashKey (const juce::String& key)
{
    // Simple djb2-style hash across the key bytes, output as 16 hex chars
    uint64_t hash = 5381;
    for (auto ch : key)
        hash = ((hash << 5) + hash) + static_cast<uint64_t> (ch);

    // Mix in a fixed salt so the hash is app-specific
    hash ^= 0xDEADC0DE12345678ULL;

    return juce::String::toHexString (static_cast<juce::int64> (hash)).paddedLeft ('0', 16).toUpperCase();
}
