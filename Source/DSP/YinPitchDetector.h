#pragma once
#include <JuceHeader.h>

class YinPitchDetector
{
public:
    void prepare(double sampleRate);
    float process(const float* samples, int numSamples, float minHz = 65.0f, float maxHz = 1100.0f);
    float getConfidence() const noexcept { return confidence; }
private:
    double sr = 48000.0;
    std::array<float, 2048> work{};
    std::array<float, 1025> diff{};
    float confidence = 0.0f;
};
