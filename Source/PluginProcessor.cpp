#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    juce::StringArray presetNames {
        "Natural Tune", "Anime Soft", "Anime Bright", "Chibi / Cute", "Cyber Idol",
        "Hard Tune (Anime Robot)", "Whisper Anime", "Male->Female Natural", "Male->Female Airy", "Male->Female Idol"
    };
}

AnimeTuneAudioProcessor::AnimeTuneAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                   .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "STATE", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout AnimeTuneAudioProcessor::createParameterLayout()
{
    using P = juce::AudioParameterFloat;
    using C = juce::AudioParameterChoice;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back(std::make_unique<C>("preset", "Preset", presetNames, 0));
    p.push_back(std::make_unique<C>("key", "Key", juce::StringArray{"Chromatic","C","C#","D","D#","E","F","F#","G","G#","A","A#","B"}, 0));
    p.push_back(std::make_unique<P>("retune", "Retune Speed", juce::NormalisableRange<float>(0.0f, 400.0f, 0.1f), 75.0f));
    p.push_back(std::make_unique<P>("strength", "Correction Strength", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.65f));
    p.push_back(std::make_unique<P>("formant", "Formant Shift", juce::NormalisableRange<float>(-0.35f, 0.80f, 0.001f), 0.12f));
    p.push_back(std::make_unique<P>("pitch", "Pitch Shift", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.01f), 0.0f));
    p.push_back(std::make_unique<P>("brightness", "Brightness", juce::NormalisableRange<float>(-6.0f, 9.0f, 0.01f), 0.0f));
    p.push_back(std::make_unique<P>("deesser", "De-esser", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.35f));
    p.push_back(std::make_unique<P>("compress", "Compression", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.25f));
    p.push_back(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false));
    return { p.begin(), p.end() };
}

void AnimeTuneAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine.prepare(sampleRate, samplesPerBlock, getTotalNumInputChannels());
    setLatencySamples(engine.getLatencySamples());
}

void AnimeTuneAudioProcessor::releaseResources() { engine.reset(); }

bool AnimeTuneAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    const auto in = layouts.getMainInputChannelSet();
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo()) && in == out;
}

void AnimeTuneAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const bool bypass = apvts.getRawParameterValue("bypass")->load() > 0.5f;
    if (bypass) return;

    const float pitchSemitones = apvts.getRawParameterValue("pitch")->load();
    const float pitchRatio = std::pow(2.0f, pitchSemitones / 12.0f);
    const float formant = apvts.getRawParameterValue("formant")->load();
    const float strength = apvts.getRawParameterValue("strength")->load();
    const float retune = apvts.getRawParameterValue("retune")->load();
    const int preset = (int) apvts.getRawParameterValue("preset")->load();
    engine.process(buffer, pitchRatio, formant, strength, retune, preset == 5, detectedMidi, detectedConfidence);

    // Lightweight output conditioning. All coefficients are parameterized and
    // smoothed by the host-facing parameters; no heap work is required here.
    const float comp = apvts.getRawParameterValue("compress")->load();
    const float brightDb = apvts.getRawParameterValue("brightness")->load();
    const float brightGain = std::pow(10.0f, brightDb / 20.0f);
    const float threshold = 0.55f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* x = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float s = x[i] * brightGain;
            const float a = std::abs(s);
            if (comp > 0.001f && a > threshold)
                s = std::copysign(threshold + (a - threshold) / (1.0f + 3.0f * comp), s);
            x[i] = juce::jlimit(-0.98f, 0.98f, s);
        }
    }
}

void AnimeTuneAudioProcessor::applyPreset(int index)
{
    index = juce::jlimit(0, 9, index);
    apvts.getParameterAsValue("preset") = index;
    struct V { float retune, strength, formant, pitch, bright, deess, comp; };
    static const V v[] = {
        { 100, .55f, .12f, 0, 0, .30f, .20f },
        { 85, .70f, .30f, .5f, 1.0f, .38f, .24f },
        { 65, .78f, .45f, .8f, 3.0f, .48f, .26f },
        { 90, .72f, .58f, 1.5f, 2.0f, .50f, .28f },
        { 45, .86f, .40f, .5f, 2.5f, .55f, .32f },
        { 10, 1.0f, .42f, 0, 1.5f, .65f, .35f },
        { 120, .45f, .35f, .3f, 1.0f, .60f, .40f },
        { 90, .55f, .35f, 5.0f, 1.0f, .42f, .30f },
        { 105, .52f, .42f, 6.0f, 2.0f, .60f, .28f },
        { 35, .85f, .47f, 6.0f, 3.0f, .72f, .34f }
    };
    const auto& q = v[index];
    apvts.getParameterAsValue("retune") = q.retune;
    apvts.getParameterAsValue("strength") = q.strength;
    apvts.getParameterAsValue("formant") = q.formant;
    apvts.getParameterAsValue("pitch") = q.pitch;
    apvts.getParameterAsValue("brightness") = q.bright;
    apvts.getParameterAsValue("deesser") = q.deess;
    apvts.getParameterAsValue("compress") = q.comp;
}

juce::String AnimeTuneAudioProcessor::getPresetName() const
{
    const int i = (int) apvts.getRawParameterValue("preset")->load();
    return presetNames[juce::jlimit(0, presetNames.size() - 1, i)];
}

void AnimeTuneAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void AnimeTuneAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* AnimeTuneAudioProcessor::createEditor()
{
    return new AnimeTuneAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AnimeTuneAudioProcessor();
}
