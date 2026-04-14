# Ducktane

A polyphonic VST3/AU software synthesizer built with JUCE/C++, inspired by Xfer Serum and NI Massive.

## Features

- Dual oscillators with up to 7-voice unison, polyBLEP anti-aliasing
- Sub oscillator (-1 / -2 octave)
- Cytomic TPT state-variable filter (LP12, LP24, HP, BP)
- Amp + mod ADSR envelopes
- 2× LFOs with multiple shapes and targets
- FX chain: Distortion → Phaser → Chorus → Delay → Reverb → Bitcrusher
- Real-time oscilloscope waveform display
- Preset browser with categories (Bass, Lead, Pad, Keys, Stab, Soundscape…)
- Preset import/export — share presets as `.xml` files
- 8 factory presets included (auto-loaded on first launch)

## Formats

| Platform | Formats |
|----------|---------|
| Windows  | VST3    |
| macOS    | VST3, AU (universal binary — Intel + Apple Silicon) |

## Download

Go to [Actions](../../actions) → latest run → **Artifacts** to download the installer for your platform.

## Building locally

Requirements: CMake 3.22+, Visual Studio 2022 (Windows) or Xcode (macOS), JUCE 8.0.12.

```bash
git clone https://github.com/pradashades333/Ducktane.git
cd Ducktane
git clone --depth 1 --branch 8.0.12 https://github.com/juce-framework/JUCE.git JUCE
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target MassiveSynth_VST3
```

> On macOS, replace `MassiveSynth_VST3` with `MassiveSynth_AU` to build the Audio Unit instead.
