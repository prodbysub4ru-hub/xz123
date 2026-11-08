#pragma once
#include <JuceHeader.h>
#include "DSP/LowLatencyVoiceEngine.h"

class AnimeTuneAudioProcessor : public juce::AudioProcessor
{
public:
    AnimeTuneAudioProcessor();
    ~AnimeTuneAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "AnimeTune"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    bool supportsDoublePrecisionProcessing() const override { return false; }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts;
    void applyPreset(int index);
    juce::String getPresetName() const;
    float getDetectedMidi() const noexcept { return detectedMidi; }
    float getDetectedConfidence() const noexcept { return detectedConfidence; }
    int getLatencySamples() const noexcept { return engine.getLatencySamples(); }

private:
    LowLatencyVoiceEngine engine;
    float detectedMidi = 0.0f;
    float detectedConfidence = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnimeTuneAudioProcessor)
};
