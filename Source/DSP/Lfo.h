#pragma once
#include <JuceHeader.h>
#include <cmath>

/**
 * Simple LFO — header-only, no DSP thread state shared externally.
 * Caller advances it per-block via tick(numSamples); returns value in [-1, +1].
 */
class Lfo
{
public:
    enum class Shape { Sine = 0, Triangle, Saw, Square, SampleAndHold };

    void prepare (double sr) noexcept { sampleRate = sr; }

    void setShape (int s)   noexcept { shape = static_cast<Shape> (s); }
    void setRate  (float hz) noexcept { rate  = hz; }
    void reset    ()         noexcept { phase = 0.0; shValue = 0.0f; }

    /** Advance by numSamples and return the LFO value at the end of that block. */
    float tick (int numSamples) noexcept
    {
        double inc = static_cast<double> (rate) * numSamples / sampleRate;
        double newPhase = phase + inc;
        // Detect wrap — used for S&H
        bool wrapped = newPhase >= 1.0;
        while (newPhase >= 1.0) newPhase -= 1.0;
        phase = newPhase;

        if (shape == Shape::SampleAndHold && wrapped)
            shValue = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;

        return currentValue();
    }

    float currentValue() const noexcept
    {
        auto p = static_cast<float> (phase);
        switch (shape)
        {
            case Shape::Sine:         return std::sin (juce::MathConstants<float>::twoPi * p);
            case Shape::Triangle:     return p < 0.5f ? (4.0f * p - 1.0f) : (3.0f - 4.0f * p);
            case Shape::Saw:          return 2.0f * p - 1.0f;
            case Shape::Square:       return p < 0.5f ? 1.0f : -1.0f;
            case Shape::SampleAndHold: return shValue;
        }
        return 0.0f;
    }

private:
    double sampleRate = 44100.0;
    float  rate       = 1.0f;
    Shape  shape      = Shape::Sine;
    double phase      = 0.0;
    float  shValue    = 0.0f;
};
