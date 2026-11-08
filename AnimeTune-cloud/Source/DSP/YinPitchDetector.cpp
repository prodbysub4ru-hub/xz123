#include "YinPitchDetector.h"

void YinPitchDetector::prepare(double sampleRate) { sr = sampleRate; confidence = 0.0f; }

float YinPitchDetector::process(const float* samples, int numSamples, float minHz, float maxHz)
{
    const int n = juce::jmin(numSamples, (int)work.size());
    if (n < 256) { confidence = 0.0f; return 0.0f; }
    std::copy(samples, samples + n, work.begin());
    const int tauMin = juce::jmax(2, (int)std::floor(sr / maxHz));
    const int tauMax = juce::jmin(n / 2 - 1, (int)std::ceil(sr / minHz));
    if (tauMax <= tauMin) { confidence = 0.0f; return 0.0f; }

    double running = 0.0;
    for (int tau = 1; tau <= tauMax; ++tau)
    {
        double d = 0.0;
        for (int i = 0; i < n - tau; ++i)
        {
            const float x = work[(size_t)i] - work[(size_t)(i + tau)];
            d += x * x;
        }
        diff[(size_t)tau] = (float)d;
    }

    float best = 1.0f;
    int bestTau = -1;
    for (int tau = 1; tau <= tauMax; ++tau)
    {
        running += diff[(size_t)tau];
        if (tau < tauMin) continue;
        const float cmnd = running > 1.0e-12 ? diff[(size_t)tau] * (float)tau / (float)running : 1.0f;
        if (cmnd < best) { best = cmnd; bestTau = tau; }
    }
    if (bestTau < 0 || best > 0.35f) { confidence = 0.0f; return 0.0f; }

    float tau = (float)bestTau;
    if (bestTau > tauMin && bestTau < tauMax)
    {
        const float a = diff[(size_t)bestTau - 1], b = diff[(size_t)bestTau], c = diff[(size_t)bestTau + 1];
        const float denom = a - 2.0f*b + c;
        if (std::abs(denom) > 1.0e-9f) tau += 0.5f * (a - c) / denom;
    }
    confidence = juce::jlimit(0.0f, 1.0f, 1.0f - best / 0.35f);
    return tau > 0.0f ? (float)(sr / tau) : 0.0f;
}
