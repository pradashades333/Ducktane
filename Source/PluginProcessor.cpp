#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DSP/SynthVoice.h"
#include "DSP/SynthSound.h"

// ============================================================
// Parameter layout (Step 2)
// ============================================================
juce::AudioProcessorValueTreeState::ParameterLayout
MassiveSynthAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // ---- Choice string arrays ----
    juce::StringArray oscShapes   { "Sine", "Triangle", "Saw", "Square" };
    juce::StringArray filterTypes { "LP 12dB", "LP 24dB", "HP", "BP" };
    juce::StringArray lfoShapes   { "Sine", "Triangle", "Saw", "Square", "S&H" };
    juce::StringArray lfoTargets  { "None", "Filter Cutoff",
                                    "OSC 1+2 Pitch", "OSC 1 Pitch", "OSC 2 Pitch" };
    juce::StringArray subOctaves  { "-2 Oct", "-1 Oct" };

    // ---- Common ranges ----
    auto adsrRange = juce::NormalisableRange<float> (0.001f, 5.0f, 0.001f, 0.35f);
    auto zeroOne   = juce::NormalisableRange<float> (0.0f,   1.0f, 0.001f);

    // ========================================================
    // OSCILLATOR 1
    // ========================================================
    layout.add (std::make_unique<juce::AudioParameterChoice> ("osc1_shape",
        "OSC 1 Shape", oscShapes, 2 /* Saw */));

    layout.add (std::make_unique<juce::AudioParameterInt> ("osc1_octave",
        "OSC 1 Octave", -2, 2, 0));

    layout.add (std::make_unique<juce::AudioParameterInt> ("osc1_semi",
        "OSC 1 Semitones", -12, 12, 0));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("osc1_fine",
        "OSC 1 Fine",
        juce::NormalisableRange<float> (-100.0f, 100.0f, 0.1f), 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("osc1_level",
        "OSC 1 Level", zeroOne, 0.8f));

    layout.add (std::make_unique<juce::AudioParameterInt> ("osc1_unison_voices",
        "OSC 1 Unison Voices", 1, 7, 1));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("osc1_unison_detune",
        "OSC 1 Unison Detune",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 20.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("osc1_unison_spread",
        "OSC 1 Unison Spread", zeroOne, 0.5f));

    // ========================================================
    // OSCILLATOR 2
    // ========================================================
    layout.add (std::make_unique<juce::AudioParameterChoice> ("osc2_shape",
        "OSC 2 Shape", oscShapes, 2));

    layout.add (std::make_unique<juce::AudioParameterInt> ("osc2_octave",
        "OSC 2 Octave", -2, 2, 0));

    layout.add (std::make_unique<juce::AudioParameterInt> ("osc2_semi",
        "OSC 2 Semitones", -12, 12, -12));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("osc2_fine",
        "OSC 2 Fine",
        juce::NormalisableRange<float> (-100.0f, 100.0f, 0.1f), 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("osc2_level",
        "OSC 2 Level", zeroOne, 0.0f));

    layout.add (std::make_unique<juce::AudioParameterInt> ("osc2_unison_voices",
        "OSC 2 Unison Voices", 1, 7, 1));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("osc2_unison_detune",
        "OSC 2 Unison Detune",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 20.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("osc2_unison_spread",
        "OSC 2 Unison Spread", zeroOne, 0.5f));

    // ========================================================
    // SUB OSCILLATOR
    // ========================================================
    layout.add (std::make_unique<juce::AudioParameterFloat> ("sub_level",
        "Sub Level", zeroOne, 0.0f));

    layout.add (std::make_unique<juce::AudioParameterChoice> ("sub_octave",
        "Sub Octave", subOctaves, 0 /* -2 Oct */));

    // ========================================================
    // FILTER
    // ========================================================
    layout.add (std::make_unique<juce::AudioParameterChoice> ("filter_type",
        "Filter Type", filterTypes, 0 /* LP 12dB */));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("filter_cutoff",
        "Filter Cutoff",
        juce::NormalisableRange<float> (20.0f, 20000.0f, 0.1f, 0.25f), 8000.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("filter_res",
        "Filter Resonance", zeroOne, 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("filter_drive",
        "Filter Drive", zeroOne, 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("filter_env_amt",
        "Filter Env Amount",
        juce::NormalisableRange<float> (-1.0f, 1.0f, 0.01f), 0.0f));

    // ========================================================
    // AMP ENVELOPE
    // ========================================================
    layout.add (std::make_unique<juce::AudioParameterFloat> ("amp_attack",
        "Amp Attack",   adsrRange, 0.005f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("amp_decay",
        "Amp Decay",    adsrRange, 0.2f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("amp_sustain",
        "Amp Sustain",  zeroOne,   0.7f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("amp_release",
        "Amp Release",  adsrRange, 0.4f));

    // ========================================================
    // MOD ENVELOPE
    // ========================================================
    layout.add (std::make_unique<juce::AudioParameterFloat> ("mod_attack",
        "Mod Attack",   adsrRange, 0.01f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("mod_decay",
        "Mod Decay",    adsrRange, 0.5f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("mod_sustain",
        "Mod Sustain",  zeroOne,   0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("mod_release",
        "Mod Release",  adsrRange, 0.2f));

    // ========================================================
    // LFO 1
    // ========================================================
    layout.add (std::make_unique<juce::AudioParameterChoice> ("lfo1_shape",
        "LFO 1 Shape", lfoShapes, 0 /* Sine */));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("lfo1_rate",
        "LFO 1 Rate",
        juce::NormalisableRange<float> (0.01f, 20.0f, 0.01f, 0.4f), 1.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("lfo1_depth",
        "LFO 1 Depth", zeroOne, 0.0f));

    layout.add (std::make_unique<juce::AudioParameterBool> ("lfo1_sync",
        "LFO 1 Sync", false));

    layout.add (std::make_unique<juce::AudioParameterChoice> ("lfo1_target",
        "LFO 1 Target", lfoTargets, 1 /* Filter Cutoff */));

    // ========================================================
    // LFO 2
    // ========================================================
    layout.add (std::make_unique<juce::AudioParameterChoice> ("lfo2_shape",
        "LFO 2 Shape", lfoShapes, 0));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("lfo2_rate",
        "LFO 2 Rate",
        juce::NormalisableRange<float> (0.01f, 20.0f, 0.01f, 0.4f), 0.5f));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("lfo2_depth",
        "LFO 2 Depth", zeroOne, 0.0f));

    layout.add (std::make_unique<juce::AudioParameterBool> ("lfo2_sync",
        "LFO 2 Sync", false));

    layout.add (std::make_unique<juce::AudioParameterChoice> ("lfo2_target",
        "LFO 2 Target", lfoTargets, 0 /* None */));

    // ========================================================
    // FX
    // ========================================================
    layout.add (std::make_unique<juce::AudioParameterFloat> ("dist_drive",
        "Dist Drive",  zeroOne, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("dist_mix",
        "Dist Mix",    zeroOne, 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("phaser_rate",
        "Phaser Rate",
        juce::NormalisableRange<float> (0.05f, 8.0f, 0.01f, 0.4f), 0.5f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("phaser_depth",
        "Phaser Depth", zeroOne, 0.5f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("phaser_mix",
        "Phaser Mix",   zeroOne, 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("crush_bits",
        "Crush Bits",
        juce::NormalisableRange<float> (2.0f, 16.0f, 0.1f), 16.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("crush_mix",
        "Crush Mix", zeroOne, 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("chorus_rate",
        "Chorus Rate",
        juce::NormalisableRange<float> (0.1f, 5.0f, 0.01f), 1.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("chorus_depth",
        "Chorus Depth", zeroOne, 0.3f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("chorus_mix",
        "Chorus Mix",   zeroOne, 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("delay_time",
        "Delay Time",
        juce::NormalisableRange<float> (0.0f, 1000.0f, 1.0f), 375.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("delay_feedback",
        "Delay Feedback",
        juce::NormalisableRange<float> (0.0f, 0.95f, 0.01f), 0.4f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("delay_mix",
        "Delay Mix", zeroOne, 0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("reverb_room",
        "Reverb Room", zeroOne, 0.5f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("reverb_damp",
        "Reverb Damp", zeroOne, 0.5f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("reverb_mix",
        "Reverb Mix",  zeroOne, 0.0f));

    // ========================================================
    // MASTER
    // ========================================================
    layout.add (std::make_unique<juce::AudioParameterFloat> ("master_volume",
        "Master Volume", zeroOne, 0.7f));

    return layout;
}

// ============================================================
// Constructor / Destructor
// ============================================================
MassiveSynthAudioProcessor::MassiveSynthAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts  (*this, nullptr, "Parameters", createParameterLayout()),
      fxChain (apvts)
{
    // Cache LFO + master volume pointers
    auto g = [&] (const char* id) { return apvts.getRawParameterValue (id); };
    pLfo1Shape    = g ("lfo1_shape");
    pLfo1Rate     = g ("lfo1_rate");
    pLfo1Sync     = g ("lfo1_sync");
    pLfo2Shape    = g ("lfo2_shape");
    pLfo2Rate     = g ("lfo2_rate");
    pLfo2Sync     = g ("lfo2_sync");
    pMasterVolume = g ("master_volume");

    // Create polyphonic voices
    for (int i = 0; i < kNumVoices; ++i)
        synth.addVoice (new SynthVoice (apvts));

    synth.addSound (new SynthSound());
}

MassiveSynthAudioProcessor::~MassiveSynthAudioProcessor() {}

// ============================================================
// prepareToPlay
// ============================================================
void MassiveSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<SynthVoice*> (synth.getVoice (i)))
            v->prepareToPlay (sampleRate, samplesPerBlock, 2);

    lfo1.prepare (sampleRate);
    lfo2.prepare (sampleRate);

    fxChain.prepare (sampleRate, samplesPerBlock);
}

void MassiveSynthAudioProcessor::releaseResources() {}

bool MassiveSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

// ============================================================
// processBlock
// ============================================================
void MassiveSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    const int numSamples = buffer.getNumSamples();

    // --- Fetch host BPM for LFO sync ---
    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
            if (auto b = pos->getBpm(); b.hasValue() && *b > 0.0)
                bpm = *b;
    }

    // --- Update and advance LFOs ---
    lfo1.setShape (static_cast<int> (pLfo1Shape->load()));
    lfo2.setShape (static_cast<int> (pLfo2Shape->load()));

    // If sync is on, convert note division to Hz; otherwise use Hz directly.
    // Sync note values: 1/4 bar → bpm/240, 1/2 → bpm/120, 1 → bpm/60, etc.
    // For now, rate parameter doubles as Hz when not synced and as a divisor index when synced.
    // Full sync implementation deferred to UI step; use raw Hz for now.
    lfo1.setRate (pLfo1Rate->load());
    lfo2.setRate (pLfo2Rate->load());

    const float lfo1Val = lfo1.tick (numSamples);
    const float lfo2Val = lfo2.tick (numSamples);

    // --- Pass mod values to every voice before rendering ---
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<SynthVoice*> (synth.getVoice (i)))
            v->setModulationValues (lfo1Val, lfo2Val);

    // --- Render all active voices ---
    synth.renderNextBlock (buffer, midiMessages, 0, numSamples);

    // --- Update UI activity flag + note frequency for scope ---
    bool anyActive = false;
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (synth.getVoice (i)->isVoiceActive())
        {
            if (! anyActive)
            {
                // Grab frequency from the first active voice
                if (auto* v = dynamic_cast<SynthVoice*> (synth.getVoice (i)))
                    scopeBuffer.noteFrequency.store (v->getCurrentFrequency(),
                                                    std::memory_order_relaxed);
            }
            anyActive = true;
        }
    }
    isAudioActive.store (anyActive);
    scopeBuffer.isActive.store (anyActive, std::memory_order_relaxed);

    // --- FX chain ---
    fxChain.process (buffer);

    // --- Master volume ---
    buffer.applyGain (pMasterVolume->load());

    // --- Push left-channel post-FX audio into scope ring buffer ---
    scopeBuffer.sampleRate.store (static_cast<float> (getSampleRate()),
                                  std::memory_order_relaxed);
    if (buffer.getNumChannels() > 0)
        scopeBuffer.push (buffer.getReadPointer (0), numSamples);
}

// ============================================================
// State persistence (preset save/load)
// ============================================================
void MassiveSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MassiveSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

// ============================================================
// Plugin entry point
// ============================================================
juce::AudioProcessorEditor* MassiveSynthAudioProcessor::createEditor()
{
    return new MassiveSynthAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MassiveSynthAudioProcessor();
}
