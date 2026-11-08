#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class AnimeTuneAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      private juce::Timer
{
public:
    explicit AnimeTuneAudioProcessorEditor(AnimeTuneAudioProcessor&);
    ~AnimeTuneAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    AnimeTuneAudioProcessor& processor;
    juce::ComboBox presetBox, keyBox;
    juce::ToggleButton bypass;
    juce::Slider retune, strength, formant, pitch, brightness, deesser, compress;
    juce::Label pitchLabel, confLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> presetAtt, keyAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> retuneAtt, strengthAtt, formantAtt, pitchAtt, brightAtt, deessAtt, compAtt;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnimeTuneAudioProcessorEditor)
};
