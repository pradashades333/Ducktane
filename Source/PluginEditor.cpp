#include "PluginEditor.h"

// Window dimensions
static constexpr int kW  = 1320;
static constexpr int kH  = 720;
static constexpr int kTopBarH = 52;
static constexpr int kFxH     = 185;
static constexpr int kMainH   = kH - kTopBarH - kFxH; // 483

MassiveSynthAudioProcessorEditor::MassiveSynthAudioProcessorEditor (MassiveSynthAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor  (p),
      presetManager   (p.apvts),
      presetPanel     (p.apvts, presetManager),
      osc1Panel       (p.apvts, "osc1_", "OSC 1"),
      osc2Panel       (p.apvts, "osc2_", "OSC 2"),
      filterPanel     (p.apvts),
      ampEnvPanel     (p.apvts, "amp_", "AMP ENV"),
      modEnvPanel     (p.apvts, "mod_", "MOD ENV"),
      lfo1Panel       (p.apvts, "lfo1_", "LFO 1"),
      lfo2Panel       (p.apvts, "lfo2_", "LFO 2"),
      fxPanel         (p.apvts),
      browserPanel    (presetManager)
{
    setLookAndFeel (&laf);

    addAndMakeVisible (presetPanel);
    addAndMakeVisible (osc1Panel);

    // Browser overlay — on top of everything, starts hidden
    addChildComponent (browserPanel);

    // Wire preset panel's browse button
    presetPanel.onBrowseClicked = [this]
    {
        browserPanel.refresh();
        browserPanel.setVisible (true);
        browserPanel.toFront (false);
    };

    // Wire browser close / load callbacks
    browserPanel.onClose = [this]
    {
        browserPanel.setVisible (false);
    };
    browserPanel.onPresetLoaded = [this]
    {
        presetPanel.refreshList();
    };
    addAndMakeVisible (osc2Panel);
    addAndMakeVisible (filterPanel);
    addAndMakeVisible (ampEnvPanel);
    addAndMakeVisible (modEnvPanel);
    addAndMakeVisible (lfo1Panel);
    addAndMakeVisible (lfo2Panel);
    addAndMakeVisible (fxPanel);

    // Give each waveform display a pointer to the processor's scope ring buffer
    osc1Panel.getWaveDisplay().setScopeBuffer (&p.scopeBuffer);
    osc2Panel.getWaveDisplay().setScopeBuffer (&p.scopeBuffer);

    setSize (kW, kH);
    setResizable (false, false);
}

MassiveSynthAudioProcessorEditor::~MassiveSynthAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void MassiveSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (MSColors::bg);

    // Subtle horizontal gradient across the main area
    const juce::Rectangle<float> mainArea (0.0f, static_cast<float> (kTopBarH),
                                           static_cast<float> (kW),
                                           static_cast<float> (kMainH));
    juce::ColourGradient grad (MSColors::bg, 0.0f, static_cast<float> (kTopBarH),
                               MSColors::panelBg.darker (0.2f),
                               static_cast<float> (kW), static_cast<float> (kTopBarH + kMainH),
                               false);
    g.setGradientFill (grad);
    g.fillRect (mainArea);
}

void MassiveSynthAudioProcessorEditor::resized()
{
    // ---- Top bar ----
    presetPanel.setBounds (0, 0, kW, kTopBarH);

    // ---- Main columns ----
    // OSC1 | OSC2 | Filter | Right (Env + LFO)
    // Widths: 240 | 240 | 215 | 405 = 1100
    const int y       = kTopBarH;
    const int h       = kMainH;
    const int oscW    = 240;
    const int filterW = 215;
    const int rightW  = kW - oscW * 2 - filterW; // 625

    osc1Panel  .setBounds (0,            y, oscW,    h);
    osc2Panel  .setBounds (oscW,         y, oscW,    h);
    filterPanel.setBounds (oscW * 2,     y, filterW, h);

    // Right column: amp env | mod env | lfo1 | lfo2
    const int rx    = oscW * 2 + filterW;
    const int envH  = h / 2;              // 241 each for both envs together
    const int lfoH  = h - envH;           // 242 for both LFOs

    const int envHalf = envH / 2;         // ~120 each
    const int lfoHalf = lfoH / 2;         // ~121 each

    ampEnvPanel.setBounds (rx, y,                    rightW, envHalf);
    modEnvPanel.setBounds (rx, y + envHalf,          rightW, envHalf);
    lfo1Panel  .setBounds (rx, y + envH,             rightW, lfoHalf);
    lfo2Panel  .setBounds (rx, y + envH + lfoHalf,   rightW, lfoH - lfoHalf);

    // ---- FX strip ----
    fxPanel.setBounds (0, kTopBarH + kMainH, kW, kFxH);

    // ---- Browser overlay (full editor, floats on top) ----
    browserPanel.setBounds (getLocalBounds());
}
