#pragma once
#include <JuceHeader.h>
#include "YinPitchDetector.h"

class LowLatencyVoiceEngine
{
public:
    static constexpr int FFTOrder = 10;
    static constexpr int FFTSize = 1 << FFTOrder;
    static constexpr int MaxBins = FFTSize / 2 + 1;

    void prepare(double sampleRate, int maxBlockSize, int channels);
    void reset();
    void process(juce::AudioBuffer<float>& buffer, float pitchRatio, float formantPercent,
                 float correctionStrength, float retuneMs, float& detectedMidi, float& detectedConfidence);
    int getLatencySamples() const noexcept { return FFTSize / 2; }

private:
    float smoothPitch(float target, float retuneMs);
    void processFrame(float* data, int n, float pitchRatio, float formantPercent, float wet);

    double sr = 48000.0;
    juce::dsp::FFT fft{ FFTOrder };
    std::array<float, FFTSize> window{};
    std::array<float, FFTSize * 2> spec{};
    std::array<float, FFTSize * 2> outSpec{};
    std::array<float, FFTSize> frame{};
    YinPitchDetector yin;
    float smoothedRatio = 1.0f;
    float lastMidi = 0.0f;
    float lastConfidence = 0.0f;
};
