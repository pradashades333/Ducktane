#include "SynthVoice.h"
#include "SynthSound.h"
#include "AdsrEnvelope.h"

// ============================================================
// Construction — cache all APVTS pointers
// ============================================================
SynthVoice::SynthVoice (juce::AudioProcessorValueTreeState& apvts)
{
    auto g = [&] (const char* id) { return apvts.getRawParameterValue (id); };

    pOsc1Shape      = g ("osc1_shape");
    pOsc1Octave     = g ("osc1_octave");
    pOsc1Semi       = g ("osc1_semi");
    pOsc1Fine       = g ("osc1_fine");
    pOsc1Level      = g ("osc1_level");
    pOsc1UniVoices  = g ("osc1_unison_voices");
    pOsc1UniDetune  = g ("osc1_unison_detune");
    pOsc1UniSpread  = g ("osc1_unison_spread");

    pOsc2Shape      = g ("osc2_shape");
    pOsc2Octave     = g ("osc2_octave");
    pOsc2Semi       = g ("osc2_semi");
    pOsc2Fine       = g ("osc2_fine");
    pOsc2Level      = g ("osc2_level");
    pOsc2UniVoices  = g ("osc2_unison_voices");
    pOsc2UniDetune  = g ("osc2_unison_detune");
    pOsc2UniSpread  = g ("osc2_unison_spread");

    pSubLevel       = g ("sub_level");
    pSubOctave      = g ("sub_octave");

    pFilterType     = g ("filter_type");
    pFilterCutoff   = g ("filter_cutoff");
    pFilterRes      = g ("filter_res");
    pFilterDrive    = g ("filter_drive");
    pFilterEnvAmt   = g ("filter_env_amt");

    pAmpAttack      = g ("amp_attack");
    pAmpDecay       = g ("amp_decay");
    pAmpSustain     = g ("amp_sustain");
    pAmpRelease     = g ("amp_release");

    pModAttack      = g ("mod_attack");
    pModDecay       = g ("mod_decay");
    pModSustain     = g ("mod_sustain");
    pModRelease     = g ("mod_release");

    pLfo1Target     = g ("lfo1_target");
    pLfo1Depth      = g ("lfo1_depth");
    pLfo2Target     = g ("lfo2_target");
    pLfo2Depth      = g ("lfo2_depth");
}

// ============================================================
// Preparation
// ============================================================
void SynthVoice::prepareToPlay (double sr, int /*samplesPerBlock*/, int /*numChannels*/)
{
    sampleRate = sr;
    osc1.prepare   (sr);
    osc2.prepare   (sr);
    subOsc.prepare (sr);
    svFilter.prepare (sr);
    ampEnv.setSampleRate (sr);
    modEnv.setSampleRate (sr);
    isPrepared = true;
}

// ============================================================
// SynthesiserVoice interface
// ============================================================
bool SynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*> (sound) != nullptr;
}

void SynthVoice::startNote (int midiNoteNumber, float velocity,
                             juce::SynthesiserSound*, int pitchWheelPosition)
{
    currentFreq     = static_cast<float> (juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber));
    currentVelocity = velocity;
    pitchWheelSemis = (static_cast<float> (pitchWheelPosition) / 8191.5f - 1.0f) * 2.0f;

    // Randomise phase on each note trigger to avoid supersaw phase cancellation.
    osc1.reset();
    osc2.reset();
    subOsc.reset();
    svFilter.reset();

    updateEnvelopes();
    ampEnv.noteOn();
    modEnv.noteOn();
}

void SynthVoice::stopNote (float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff && ampEnv.isActive())
    {
        ampEnv.noteOff();
        modEnv.noteOff();
    }
    else
    {
        clearCurrentNote();
        ampEnv.reset();
        modEnv.reset();
    }
}

void SynthVoice::pitchWheelMoved (int newValue)
{
    pitchWheelSemis = (static_cast<float> (newValue) / 8191.5f - 1.0f) * 2.0f;
}

// ============================================================
// Render
// ============================================================
void SynthVoice::renderNextBlock (juce::AudioBuffer<float>& buffer,
                                   int startSample, int numSamples)
{
    if (!isVoiceActive() || !isPrepared)
        return;

    // --- LFO target routing (computed once per block) ---
    // Target indices: 0=None, 1=FilterCutoff, 2=OSC1+2 Pitch, 3=OSC1 Pitch, 4=OSC2 Pitch
    const int   lfo1Target = static_cast<int> (pLfo1Target->load());
    const float lfo1Depth  = pLfo1Depth->load();
    const int   lfo2Target = static_cast<int> (pLfo2Target->load());
    const float lfo2Depth  = pLfo2Depth->load();

    // Pitch LFO (semitones, ±24 max)
    float pitchLfo = 0.0f;
    if (lfo1Target == 2 || lfo1Target == 3)
        pitchLfo += lfo1Value * lfo1Depth * 24.0f;
    if (lfo2Target == 2 || lfo2Target == 3)
        pitchLfo += lfo2Value * lfo2Depth * 24.0f;

    // Include pitch wheel
    pitchLfo += pitchWheelSemis;

    // Filter LFO (normalised 0→1 mapped to ±2 octaves via log2)
    float filterLfo = 0.0f;
    if (lfo1Target == 1) filterLfo += lfo1Value * lfo1Depth;
    if (lfo2Target == 1) filterLfo += lfo2Value * lfo2Depth;

    // --- Update oscillators and ADSR parameters (per block) ---
    updateOscillators (pitchLfo);
    updateEnvelopes();

    // --- Cache filter params for the inner loop ---
    const float baseCutoff  = pFilterCutoff->load();
    const float filterEnvAmt = pFilterEnvAmt->load();
    svFilter.setType      (static_cast<int> (pFilterType->load()));
    svFilter.setResonance (pFilterRes->load());
    svFilter.setDrive     (pFilterDrive->load());

    // --- Per-sample loop ---
    for (int s = 0; s < numSamples; ++s)
    {
        const float ampVal = ampEnv.getNextSample();
        const float modVal = modEnv.getNextSample();

        if (!ampEnv.isActive())
        {
            clearCurrentNote();
            break;
        }

        // Cutoff: base * 2^(envAmt*4*modVal) * 2^(lfoMod*2)
        float cutoff = baseCutoff
                     * std::pow (2.0f, filterEnvAmt * 4.0f * modVal + filterLfo * 2.0f);
        cutoff = juce::jlimit (20.0f, 20000.0f, cutoff);
        svFilter.setCutoff (cutoff);

        // Oscillators
        auto [o1L, o1R]   = osc1.processSample();
        auto [o2L, o2R]   = osc2.processSample();
        auto [subL, subR] = subOsc.processSample();

        const float osc1Lv = pOsc1Level->load();
        const float osc2Lv = pOsc2Level->load();
        const float subLv  = pSubLevel->load();

        float left  = o1L * osc1Lv + o2L * osc2Lv + subL * subLv;
        float right = o1R * osc1Lv + o2R * osc2Lv + subR * subLv;

        // Filter
        left  = svFilter.processLeft  (left);
        right = svFilter.processRight (right);

        // Amp envelope + velocity
        left  *= ampVal * currentVelocity;
        right *= ampVal * currentVelocity;

        if (buffer.getNumChannels() > 0) buffer.addSample (0, startSample + s, left);
        if (buffer.getNumChannels() > 1) buffer.addSample (1, startSample + s, right);
    }
}

// ============================================================
// Private helpers
// ============================================================
void SynthVoice::updateOscillators (float pitchLfoSemis) noexcept
{
    // Helper: compute frequency with octave/semi/fine offsets
    auto calcFreq = [&] (float baseHz, float octave, float semi, float fine) -> float
    {
        const float semitones = octave * 12.0f + semi + fine / 100.0f + pitchLfoSemis;
        return baseHz * std::pow (2.0f, semitones / 12.0f);
    };

    // --- OSC 1 ---
    osc1.setWaveform     (static_cast<int> (pOsc1Shape->load()));
    osc1.setFrequency    (calcFreq (currentFreq,
                                    pOsc1Octave->load(),
                                    pOsc1Semi->load(),
                                    pOsc1Fine->load()));
    osc1.setUnisonVoices (static_cast<int> (pOsc1UniVoices->load()));
    osc1.setUnisonDetune (pOsc1UniDetune->load());
    osc1.setUnisonSpread (pOsc1UniSpread->load());

    // --- OSC 2 ---
    osc2.setWaveform     (static_cast<int> (pOsc2Shape->load()));
    osc2.setFrequency    (calcFreq (currentFreq,
                                    pOsc2Octave->load(),
                                    pOsc2Semi->load(),
                                    pOsc2Fine->load()));
    osc2.setUnisonVoices (static_cast<int> (pOsc2UniVoices->load()));
    osc2.setUnisonDetune (pOsc2UniDetune->load());
    osc2.setUnisonSpread (pOsc2UniSpread->load());

    // --- Sub oscillator — sine, 1 or 2 octaves below ---
    const int subOctShift = (static_cast<int> (pSubOctave->load()) == 0) ? -2 : -1;
    subOsc.setWaveform   (static_cast<int> (Oscillator::Waveform::Sine));
    subOsc.setFrequency  (currentFreq * std::pow (2.0f, static_cast<float> (subOctShift)));
    subOsc.setUnisonVoices (1);
    subOsc.setUnisonDetune (0.0f);
    subOsc.setUnisonSpread (0.0f);
}

void SynthVoice::updateEnvelopes() noexcept
{
    ampEnv.setParameters (makeAdsrParams (pAmpAttack, pAmpDecay, pAmpSustain, pAmpRelease));
    modEnv.setParameters (makeAdsrParams (pModAttack, pModDecay, pModSustain, pModRelease));
}
