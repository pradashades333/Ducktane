#include "WaveformDisplay.h"

// ============================================================
// Construction / Destruction
// ============================================================
WaveformDisplay::WaveformDisplay()
{
    startTimerHz (60);
}

WaveformDisplay::~WaveformDisplay()
{
    stopTimer();
}

// ============================================================
// 60 Hz timer
// ============================================================
void WaveformDisplay::timerCallback()
{
    if (pScope == nullptr)
        return;

    // Pull a triggered frame from the ring buffer
    const int got = pScope->readTriggered (frameBuf.data(), kDisplaySamples);
    frameValid = (got == kDisplaySamples);
    repaint();
}

// ============================================================
// Glow stroke
// ============================================================
void WaveformDisplay::strokeGlow (juce::Graphics& g, const juce::Path& path,
                                   juce::Colour colour, float alpha, float thickness)
{
    g.setColour (colour.withAlpha (alpha * 0.15f));
    g.strokePath (path, juce::PathStrokeType (thickness * 5.5f,
                  juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour (colour.withAlpha (alpha * 0.30f));
    g.strokePath (path, juce::PathStrokeType (thickness * 2.5f,
                  juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour (colour.withAlpha (alpha));
    g.strokePath (path, juce::PathStrokeType (thickness,
                  juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

// ============================================================
// Paint
// ============================================================
void WaveformDisplay::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat().reduced (1.5f);

    // ---- Background ----
    g.setColour (juce::Colour (0xff080a12));
    g.fillRoundedRectangle (bounds, 5.0f);
    g.setColour (MSColors::border);
    g.drawRoundedRectangle (bounds, 5.0f, 1.0f);

    // ---- Grid ----
    const float cy = bounds.getCentreY();
    const float q  = bounds.getHeight() * 0.25f;
    g.setColour (MSColors::textDim.withAlpha (0.20f));
    g.drawLine (bounds.getX() + 6.0f, cy, bounds.getRight() - 6.0f, cy, 0.8f);
    g.setColour (MSColors::textDim.withAlpha (0.09f));
    g.drawLine (bounds.getX() + 6.0f, cy - q, bounds.getRight() - 6.0f, cy - q, 0.6f);
    g.drawLine (bounds.getX() + 6.0f, cy + q, bounds.getRight() - 6.0f, cy + q, 0.6f);
    const float hx = bounds.getX() + bounds.getWidth() * 0.5f;
    g.drawLine (hx, bounds.getY() + 4.0f, hx, bounds.getBottom() - 4.0f, 0.6f);

    // ---- Waveform ----
    const auto waveArea = bounds.reduced (6.0f, 3.0f);
    const float w  = waveArea.getWidth();
    const float h  = waveArea.getHeight() * 0.45f;
    const float cw = waveArea.getCentreY();

    // Colour: green when silent, cyan when active
    const bool active = (pScope != nullptr) && pScope->isActive.load (std::memory_order_relaxed);
    const juce::Colour waveColour = active
        ? MSColors::green.interpolatedWith (juce::Colour (0xff00e5ff), 0.35f)
        : MSColors::green;

    if (frameValid)
    {
        // Find peak amplitude for normalisation (avoid dividing tiny noise)
        float peak = 0.0f;
        for (float s : frameBuf)
            peak = std::max (peak, std::abs (s));

        const float scale = (peak > 0.01f) ? (1.0f / peak) : 1.0f;

        juce::Path path;
        for (int i = 0; i < kDisplaySamples; ++i)
        {
            const float t = static_cast<float> (i) / (kDisplaySamples - 1);
            const float x = waveArea.getX() + t * w;
            const float y = cw - frameBuf[i] * scale * h;

            if (i == 0) path.startNewSubPath (x, y);
            else        path.lineTo (x, y);
        }

        strokeGlow (g, path, waveColour, 0.90f, 1.6f);
    }
    else
    {
        // Silent: draw a flat centre line
        juce::Path path;
        path.startNewSubPath (waveArea.getX(), cw);
        path.lineTo          (waveArea.getRight(), cw);
        strokeGlow (g, path, waveColour, 0.35f, 1.2f);
    }

    // ---- "LIVE" badge when active ----
    g.setFont (juce::Font (8.5f, juce::Font::bold));
    g.setColour (waveColour.withAlpha (active ? 0.75f : 0.35f));
    g.drawText (active ? "LIVE" : "---",
                static_cast<int> (bounds.getX()), static_cast<int> (bounds.getY()) + 3,
                50, 11, juce::Justification::centredLeft);
}
