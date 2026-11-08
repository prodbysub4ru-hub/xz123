#include "PluginEditor.h"

namespace
{
    class Look : public juce::LookAndFeel_V4
    {
    public:
        Look()
        {
            setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff090a0d));
            setColour(juce::Slider::thumbColourId, juce::Colour(0xff00e5ff));
            setColour(juce::Slider::trackColourId, juce::Colour(0xff00e5ff));
            setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff32363f));
            setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff111319));
            setColour(juce::ComboBox::textColourId, juce::Colours::white);
            setColour(juce::Label::textColourId, juce::Colour(0xffd8dbe2));
        }
    } look;

    void styleSlider(juce::Slider& s, juce::Slider::SliderStyle style = juce::Slider::LinearHorizontal)
    {
        s.setSliderStyle(style);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 68, 22);
        s.setLookAndFeel(&look);
    }
}

AnimeTuneAudioProcessorEditor::AnimeTuneAudioProcessorEditor(AnimeTuneAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&look);
    setResizable(true, true);
    setSize(620, 430);

    presetBox.addItemList(juce::StringArray{
        "Natural Tune", "Anime Soft", "Anime Bright", "Chibi / Cute", "Cyber Idol",
        "Hard Tune (Anime Robot)", "Whisper Anime", "Male->Female Natural", "Male->Female Airy", "Male->Female Idol"}, 1);
    keyBox.addItemList(juce::StringArray{"Chromatic","C","C#","D","D#","E","F","F#","G","G#","A","A#","B"}, 1);
    addAndMakeVisible(presetBox); addAndMakeVisible(keyBox); addAndMakeVisible(bypass);
    presetBox.setLookAndFeel(&look); keyBox.setLookAndFeel(&look);
    bypass.setButtonText("BYPASS");
    bypass.setLookAndFeel(&look);

    styleSlider(retune); styleSlider(strength); styleSlider(formant); styleSlider(pitch); styleSlider(brightness); styleSlider(deesser); styleSlider(compress);
    for (auto* s : {&retune,&strength,&formant,&pitch,&brightness,&deesser,&compress}) addAndMakeVisible(*s);

    auto addLabel = [this](juce::Label& l, const juce::String& t) { l.setText(t, juce::dontSendNotification); l.setJustificationType(juce::Justification::centredLeft); addAndMakeVisible(l); };
    addLabel(pitchLabel, "Pitch: —"); addLabel(confLabel, "CONF 0%   |   LATENCY");

    presetAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.apvts, "preset", presetBox);
    keyAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.apvts, "key", keyBox);
    bypassAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.apvts, "bypass", bypass);
    retuneAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "retune", retune);
    strengthAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "strength", strength);
    formantAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "formant", formant);
    pitchAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "pitch", pitch);
    brightAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "brightness", brightness);
    deessAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "deesser", deesser);
    compAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "compress", compress);
    startTimerHz(20);
}

void AnimeTuneAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff090a0d));
    g.setColour(juce::Colour(0xff00e5ff));
    g.fillRect(24, 23, 4, 38);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    g.drawText("ANIMETUNE", 38, 18, 220, 30, juce::Justification::left);
    g.setColour(juce::Colour(0xff7d828c));
    g.setFont(12.0f);
    g.drawText("LOW-LATENCY VOCAL PITCH + FORMANT", 39, 45, 300, 18, juce::Justification::left);

    g.setColour(juce::Colour(0xff1c2028));
    g.drawRoundedRectangle(24.0f, 76.0f, (float)getWidth()-48.0f, 60.0f, 8.0f, 1.0f);
    g.setColour(juce::Colour(0xff101319));
    g.drawRoundedRectangle(24.0f, 148.0f, (float)getWidth()-48.0f, 230.0f, 8.0f, 1.0f);

    g.setColour(juce::Colour(0xff7d828c)); g.setFont(11.0f);
    g.drawText("PRESET", 42, 84, 80, 18, juce::Justification::left);
    g.drawText("KEY", 290, 84, 50, 18, juce::Justification::left);
    g.drawText("RETUNE", 42, 158, 100, 18, juce::Justification::left);
    g.drawText("CORRECTION", 42, 206, 100, 18, juce::Justification::left);
    g.drawText("FORMANT", 42, 254, 100, 18, juce::Justification::left);
    g.drawText("PITCH", 42, 302, 100, 18, juce::Justification::left);
    g.drawText("BRIGHTNESS", 320, 158, 100, 18, juce::Justification::left);
    g.drawText("DE-ESSER", 320, 206, 100, 18, juce::Justification::left);
    g.drawText("COMPRESSION", 320, 254, 100, 18, juce::Justification::left);
}

void AnimeTuneAudioProcessorEditor::resized()
{
    const int w = getWidth();
    presetBox.setBounds(42, 101, 230, 28);
    keyBox.setBounds(290, 101, 120, 28);
    bypass.setBounds(w - 145, 94, 105, 34);
    retune.setBounds(145, 154, 145, 28);
    strength.setBounds(145, 202, 145, 28);
    formant.setBounds(145, 250, 145, 28);
    pitch.setBounds(145, 298, 145, 28);
    brightness.setBounds(420, 154, 145, 28);
    deesser.setBounds(420, 202, 145, 28);
    compress.setBounds(420, 250, 145, 28);
    pitchLabel.setBounds(42, 345, 220, 22);
    confLabel.setBounds(300, 345, 260, 22);
}

void AnimeTuneAudioProcessorEditor::timerCallback()
{
    const float midi = processor.getDetectedMidi();
    const int note = (int) std::round(midi);
    const int pc = ((note % 12) + 12) % 12;
    const char* names[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    if (midi > 0.0f)
        pitchLabel.setText("Pitch: " + juce::String(names[pc]) + juce::String(note / 12 - 1), juce::dontSendNotification);
    else
        pitchLabel.setText("Pitch: —", juce::dontSendNotification);
    confLabel.setText("CONF " + juce::String((int)(processor.getDetectedConfidence()*100.0f)) + "%   |   LATENCY " + juce::String(processor.getLatencySamples()) + " samples", juce::dontSendNotification);
}
