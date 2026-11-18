#pragma once

#include "../GUI/LCDDisplay.h"
#include "../GUI/LedMeter.h"
#include "../GUI/MakosizerXLookAndFeel.h"

#include <JuceHeader.h>

#include <array>
#include <memory>

class MakosizerXAudioProcessor;

class MakosizerXAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                             private juce::Slider::Listener,
                                             private juce::Timer
{
public:
    explicit MakosizerXAudioProcessorEditor (MakosizerXAudioProcessor& processor);
    ~MakosizerXAudioProcessorEditor() override;

    void paint (juce::Graphics& graphics) override;

private:
    void sliderValueChanged (juce::Slider* slider) override;
    void timerCallback() override;
    void updateLCDDisplay();

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    MakosizerXAudioProcessor& audioProcessor;
    juce::Image backgroundImage;
    MakosizerXLookAndFeel rotaryLookAndFeel;
    std::array<juce::Slider, 14> sliders;
    std::array<std::unique_ptr<SliderAttachment>, 14> attachments;
    LCDDisplay lcdDisplay;
    LedMeter ledMeter;
    float outputLevel = 0.0f;
};
