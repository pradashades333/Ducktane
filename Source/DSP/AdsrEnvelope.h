#pragma once
#include <JuceHeader.h>

/**
 * Thin header-only helper — assembles a juce::ADSR::Parameters struct
 * from four cached atomic<float>* pointers (seconds).
 */
inline juce::ADSR::Parameters makeAdsrParams (std::atomic<float>* attack,
                                               std::atomic<float>* decay,
                                               std::atomic<float>* sustain,
                                               std::atomic<float>* release) noexcept
{
    return { attack->load(), decay->load(), sustain->load(), release->load() };
}
