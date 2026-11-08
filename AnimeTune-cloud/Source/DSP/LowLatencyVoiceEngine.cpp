#include "LowLatencyVoiceEngine.h"

namespace
{
    float hzToMidi(float hz) { return 69.0f + 12.0f * std::log2(juce::jmax(hz, 1.0f) / 440.0f); }
}

void LowLatencyVoiceEngine::prepare(double sampleRate, int, int)
{
    sr = sampleRate;
    yin.prepare(sr);
    smoothedRatio = 1.0f;
    lastMidi = lastConfidence = 0.0f;
    for (int i = 0; i < FFTSize; ++i)
        window[(size_t)i] = 0.5f - 0.5f * std::cos(juce::MathConstants<double>::twoPi * i / (FFTSize - 1));
}

void LowLatencyVoiceEngine::reset()
{
    smoothedRatio = 1.0f;
    lastMidi = lastConfidence = 0.0f;
    std::fill(spec.begin(), spec.end(), 0.0f);
    std::fill(outSpec.begin(), outSpec.end(), 0.0f);
}

float LowLatencyVoiceEngine::smoothPitch(float target, float retuneMs)
{
    if (retuneMs <= 0.5f) { smoothedRatio = target; return target; }
    const float alpha = 1.0f - std::exp(-1000.0f / (float)(retuneMs * sr));
    smoothedRatio += alpha * (target - smoothedRatio);
    return smoothedRatio;
}

void LowLatencyVoiceEngine::processFrame(float* data, int n, float pitchRatio, float formantPercent, float wet)
{
    std::fill(spec.begin(), spec.end(), 0.0f);
    for (int i = 0; i < FFTSize; ++i)
        spec[(size_t)i] = (i < n ? data[i] : 0.0f) * window[(size_t)i];
    fft.performRealOnlyForwardTransform(spec.data());

    std::fill(outSpec.begin(), outSpec.end(), 0.0f);
    const int bins = FFTSize / 2;
    const float formantRatio = 1.0f + juce::jlimit(-0.30f, 0.80f, formantPercent);
    const float r = juce::jmax(0.25f, pitchRatio);

    // Harmonic remapping plus a smoothed spectral-envelope contribution. This is
    // intentionally lightweight: it keeps the vowel envelope moving independently
    // of the pitch ratio without introducing a neural model or long look-ahead.
    for (int k = 1; k < bins; ++k)
    {
        const int pitchSrc = juce::jlimit(1, bins - 1, (int)std::lround(k / r));
        const int formantSrc = juce::jlimit(1, bins - 1, (int)std::lround(k / formantRatio));
        const float pMag = std::hypot(spec[(size_t)2*pitchSrc], spec[(size_t)2*pitchSrc+1]);
        const float fMag = std::hypot(spec[(size_t)2*formantSrc], spec[(size_t)2*formantSrc+1]);
        const float local = juce::jlimit(0.35f, 2.5f, fMag / juce::jmax(pMag, 1.0e-5f));
        const float mag = juce::hypot(spec[(size_t)2*k], spec[(size_t)2*k+1]);
        const float phase = std::atan2(spec[(size_t)2*k+1], spec[(size_t)2*k]);
        const float formantBlend = juce::jlimit(0.0f, 1.0f, std::abs(formantPercent));
        const float shaped = mag * juce::jmap(formantBlend, 1.0f, local);
        const float finalMag = juce::jmap(wet, mag, shaped);
        outSpec[(size_t)2*k] = finalMag * std::cos(phase);
        outSpec[(size_t)2*k+1] = finalMag * std::sin(phase);
    }
    outSpec[0] = spec[0];
    fft.performRealOnlyInverseTransform(outSpec.data());
    const float norm = 1.0f / (float)FFTSize;
    for (int i = 0; i < n; ++i)
    {
        const float processed = outSpec[(size_t)i] * norm / juce::jmax(window[(size_t)i], 0.12f);
        data[i] = juce::jmap(wet, data[i], processed);
    }
}

void LowLatencyVoiceEngine::process(juce::AudioBuffer<float>& buffer, float pitchRatio, float formantPercent,
                                    float correctionStrength, float retuneMs, float& detectedMidi, float& detectedConfidence)
{
    const int n = buffer.getNumSamples();
    if (n <= 0) return;
    const int analysisN = juce::jmin(n, FFTSize);
    const float hz = yin.process(buffer.getReadPointer(0), analysisN, 65.0f, 1100.0f);
    const float conf = yin.getConfidence();
    if (hz > 0.0f && conf > 0.15f)
    {
        lastMidi = hzToMidi(hz);
        lastConfidence = conf;
    }
    detectedMidi = lastMidi;
    detectedConfidence = lastConfidence;

    // Target-note correction is intentionally conservative here: the exposed pitch
    // control provides explicit semitone movement; the correction stage eases toward
    // the nearest chromatic note when a confident F0 is present.
    float correctedRatio = pitchRatio;
    if (hz > 0.0f && conf > 0.15f)
    {
        const float nearest = std::round(lastMidi);
        const float noteRatio = std::pow(2.0f, (nearest - lastMidi) / 12.0f);
        correctedRatio *= std::pow(noteRatio, juce::jlimit(0.0f, 1.0f, correctionStrength));
    }
    correctedRatio = smoothPitch(correctedRatio, retuneMs);
    const float wet = juce::jlimit(0.0f, 1.0f, correctionStrength);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        const int frameN = juce::jmin(n, FFTSize);
        processFrame(data, frameN, correctedRatio, formantPercent, wet);
    }
}
