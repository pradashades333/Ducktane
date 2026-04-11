#include "FxChain.h"

// ============================================================
// Construction
// ============================================================
FxChain::FxChain (juce::AudioProcessorValueTreeState& apvts)
{
    auto g = [&] (const char* id) { return apvts.getRawParameterValue (id); };

    pDistDrive      = g ("dist_drive");
    pDistMix        = g ("dist_mix");
    pPhaserRate     = g ("phaser_rate");
    pPhaserDepth    = g ("phaser_depth");
    pPhaserMix      = g ("phaser_mix");
    pCrushBits      = g ("crush_bits");
    pCrushMix       = g ("crush_mix");
    pChorusRate     = g ("chorus_rate");
    pChorusDepth    = g ("chorus_depth");
    pChorusMix      = g ("chorus_mix");
    pDelayTime      = g ("delay_time");
    pDelayFeedback  = g ("delay_feedback");
    pDelayMix       = g ("delay_mix");
    pReverbRoom     = g ("reverb_room");
    pReverbDamp     = g ("reverb_damp");
    pReverbMix      = g ("reverb_mix");
}

// ============================================================
// Preparation
// ============================================================
void FxChain::prepare (double sr, int samplesPerBlock)
{
    sampleRate = sr;

    // Chorus buffers
    chorusBufL.assign (kChorusBufSize, 0.0f);
    chorusBufR.assign (kChorusBufSize, 0.0f);
    chorusWritePos = 0;
    chorusPhase    = 0.0;

    // Delay buffers
    delayBufL.assign (kMaxDelaySamples, 0.0f);
    delayBufR.assign (kMaxDelaySamples, 0.0f);
    delayWritePos = 0;

    // Reverb
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sr;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels      = 2;
    reverb.prepare (spec);
}

// ============================================================
// Main process call
// ============================================================
void FxChain::process (juce::AudioBuffer<float>& buffer)
{
    processDistortion (buffer, pDistDrive->load(),     pDistMix->load());
    processPhaser     (buffer, pPhaserRate->load(),   pPhaserDepth->load(), pPhaserMix->load());
    processChorus     (buffer, pChorusRate->load(),   pChorusDepth->load(), pChorusMix->load());
    processDelay      (buffer, pDelayTime->load(),    pDelayFeedback->load(), pDelayMix->load());
    processReverb     (buffer, pReverbRoom->load(),   pReverbDamp->load(),    pReverbMix->load());
    processBitcrusher (buffer, pCrushBits->load(),   pCrushMix->load());
}

// ============================================================
// Distortion — tanh soft clip, dry/wet mix
// ============================================================
void FxChain::processDistortion (juce::AudioBuffer<float>& buffer,
                                  float drive, float mix) noexcept
{
    if (mix < 0.001f) return;

    const float gain    = 1.0f + drive * 15.0f;
    const float normInv = 1.0f / std::tanh (gain);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        float* data = buffer.getWritePointer (ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float dry = data[i];
            const float wet = std::tanh (dry * gain) * normInv;
            data[i] = dry + mix * (wet - dry);
        }
    }
}

// ============================================================
// Chorus — stereo modulated delay (L: 0°, R: 90°)
// ============================================================
void FxChain::processChorus (juce::AudioBuffer<float>& buffer,
                               float rate, float depth, float mix) noexcept
{
    if (mix < 0.001f) return;

    const int   numSamples   = buffer.getNumSamples();
    const float phaseInc     = static_cast<float> (rate / sampleRate);
    // Centre delay 15 ms; modulation ±10 ms scaled by depth.
    const float centreDelay  = static_cast<float> (sampleRate * 0.015);
    const float modDepth     = static_cast<float> (sampleRate * 0.010) * depth;

    float* dataL = buffer.getWritePointer (0);
    float* dataR = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : dataL;

    for (int i = 0; i < numSamples; ++i)
    {
        const float phaseR = static_cast<float> (
            std::fmod (chorusPhase + 0.25, 1.0)); // 90° offset

        const float modL = std::sin (juce::MathConstants<float>::twoPi
                                     * static_cast<float> (chorusPhase));
        const float modR = std::sin (juce::MathConstants<float>::twoPi * phaseR);

        const float delayL = centreDelay + modL * modDepth;
        const float delayR = centreDelay + modR * modDepth;

        const float wetL = readInterp (chorusBufL, chorusWritePos, delayL);
        const float wetR = readInterp (chorusBufR, chorusWritePos, delayR);

        // Write dry signal to chorus buffer
        chorusBufL[static_cast<std::size_t> (chorusWritePos) % chorusBufL.size()] = dataL[i];
        chorusBufR[static_cast<std::size_t> (chorusWritePos) % chorusBufR.size()] = dataR[i];
        ++chorusWritePos;

        dataL[i] = dataL[i] * (1.0f - mix) + wetL * mix;
        dataR[i] = dataR[i] * (1.0f - mix) + wetR * mix;

        chorusPhase += phaseInc;
        if (chorusPhase >= 1.0) chorusPhase -= 1.0;
    }
}

// ============================================================
// Delay — ping-pong feedback delay (stereo circular buffer)
// ============================================================
void FxChain::processDelay (juce::AudioBuffer<float>& buffer,
                              float timeMs, float feedback, float mix) noexcept
{
    if (mix < 0.001f) return;

    const int numSamples    = buffer.getNumSamples();
    const int delaySamples  = juce::jlimit (1, kMaxDelaySamples - 1,
                                             static_cast<int> (sampleRate * timeMs / 1000.0));
    float* dataL = buffer.getWritePointer (0);
    float* dataR = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : dataL;

    for (int i = 0; i < numSamples; ++i)
    {
        const std::size_t sz = delayBufL.size();
        const int readPos = static_cast<int> (
            (static_cast<std::size_t> (delayWritePos) + sz
             - static_cast<std::size_t> (delaySamples)) % sz);

        const float delL = delayBufL[static_cast<std::size_t> (readPos)];
        const float delR = delayBufR[static_cast<std::size_t> (readPos)];

        // Ping-pong: L input feeds to R delay and vice versa.
        delayBufL[static_cast<std::size_t> (delayWritePos)] = dataL[i] + delR * feedback;
        delayBufR[static_cast<std::size_t> (delayWritePos)] = dataR[i] + delL * feedback;
        delayWritePos = static_cast<int> (
            (static_cast<std::size_t> (delayWritePos) + 1) % sz);

        dataL[i] = dataL[i] * (1.0f - mix) + delL * mix;
        dataR[i] = dataR[i] * (1.0f - mix) + delR * mix;
    }
}

// ============================================================
// Reverb — JUCE Freeverb wrapper
// ============================================================
void FxChain::processReverb (juce::AudioBuffer<float>& buffer,
                               float room, float damp, float mix) noexcept
{
    if (mix < 0.001f) return;
    if (buffer.getNumChannels() < 2) return;

    juce::dsp::Reverb::Parameters params;
    params.roomSize   = room;
    params.damping    = damp;
    params.wetLevel   = mix;
    params.dryLevel   = 1.0f - mix;
    params.width      = 1.0f;
    params.freezeMode = 0.0f;
    reverb.setParameters (params);

    // Process in-place using juce::dsp::AudioBlock
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);
    reverb.process (ctx);
}

// ============================================================
// Phaser — 4-stage allpass with sine LFO, feedback
// ============================================================
void FxChain::processPhaser (juce::AudioBuffer<float>& buffer,
                               float rate, float depth, float mix) noexcept
{
    if (mix < 0.001f) return;

    const int   numSamples = buffer.getNumSamples();
    const float phaseInc   = static_cast<float> (rate / sampleRate);
    // Centre frequency sweeps 200 Hz – 8 kHz driven by depth
    const float freqMin    = 200.0f;
    const float freqMax    = 8000.0f;
    const float feedback   = 0.55f;

    float* dataL = buffer.getWritePointer (0);
    float* dataR = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : dataL;

    for (int i = 0; i < numSamples; ++i)
    {
        // LFO: L at 0°, R at 90°
        const float lfoL = 0.5f + 0.5f * std::sin (
            juce::MathConstants<float>::twoPi * static_cast<float> (phaserPhase));
        const float lfoR = 0.5f + 0.5f * std::sin (
            juce::MathConstants<float>::twoPi * static_cast<float> (phaserPhase) + juce::MathConstants<float>::halfPi);

        const float fcL  = freqMin + lfoL * (freqMax - freqMin) * depth;
        const float fcR  = freqMin + lfoR * (freqMax - freqMin) * depth;

        // Allpass coefficient from bilinear transform: d = (g-1)/(g+1), g = tan(pi*fc/sr)
        const float dL = [&]
        {
            const float g = std::tan (juce::MathConstants<float>::pi * fcL / static_cast<float> (sampleRate));
            return (g - 1.0f) / (g + 1.0f);
        }();
        const float dR = [&]
        {
            const float g = std::tan (juce::MathConstants<float>::pi * fcR / static_cast<float> (sampleRate));
            return (g - 1.0f) / (g + 1.0f);
        }();

        // Process L through 4 allpass stages with feedback
        float inL = dataL[i] + phaserFbL * feedback;
        for (auto& s : phaserL)
        {
            float y = dL * (inL - s.y1) + s.x1;
            s.x1 = inL; s.y1 = y; inL = y;
        }
        phaserFbL = inL;

        float inR = dataR[i] + phaserFbR * feedback;
        for (auto& s : phaserR)
        {
            float y = dR * (inR - s.y1) + s.x1;
            s.x1 = inR; s.y1 = y; inR = y;
        }
        phaserFbR = inR;

        dataL[i] = dataL[i] * (1.0f - mix) + inL * mix;
        dataR[i] = dataR[i] * (1.0f - mix) + inR * mix;

        phaserPhase += phaseInc;
        if (phaserPhase >= 1.0) phaserPhase -= 1.0;
    }
}

// ============================================================
// Bitcrusher — bit-depth quantisation (no SRR, avoids DC issues)
// ============================================================
void FxChain::processBitcrusher (juce::AudioBuffer<float>& buffer,
                                   float bits, float mix) noexcept
{
    if (mix < 0.001f || bits >= 15.9f) return;

    const float levels = std::pow (2.0f, juce::jlimit (2.0f, 16.0f, bits)) - 1.0f;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        float* data = buffer.getWritePointer (ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float dry = data[i];
            const float wet = std::round (dry * levels) / levels;
            data[i] = dry + mix * (wet - dry);
        }
    }
}

// ============================================================
// Linear interpolation into a circular buffer
// ============================================================
float FxChain::readInterp (const std::vector<float>& buf,
                             int writePos, float delaySamples) noexcept
{
    const int sz = static_cast<int> (buf.size());
    float readF  = static_cast<float> (writePos) - delaySamples;
    while (readF < 0.0f) readF += static_cast<float> (sz);

    const int   r0   = static_cast<int> (readF) % sz;
    const int   r1   = (r0 + 1) % sz;
    const float frac = readF - std::floor (readF);
    return buf[static_cast<std::size_t> (r0)] * (1.0f - frac)
         + buf[static_cast<std::size_t> (r1)] * frac;
}
