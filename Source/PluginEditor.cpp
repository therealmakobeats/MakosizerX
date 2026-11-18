#include "PluginEditor.h"

#include "PluginProcessor.h"

#include <array>
#include <cstddef>

namespace
{
struct ControlPlacement
{
    const char* parameterId;
    const char* label;
    int x;
    int y;
    int width;
    int height;
};

// LOCKED: Native-artwork coordinates. Layout changes require an explicit design revision.
constexpr std::array<ControlPlacement, 14> controlPlacements {{
    { "drive", "Drive", 298, 367, 115, 110 },
    { "vinyl", "Vinyl", 469, 367, 115, 110 },
    { "destroy", "Destroy", 637, 364, 240, 229 },
    { "wow", "Wow", 946, 367, 115, 110 },
    { "flutter", "Flutter", 1116, 367, 115, 110 },
    { "noise", "Noise", 296, 575, 115, 110 },
    { "dust", "Dust", 470, 575, 115, 110 },
    { "crush", "Crush", 943, 575, 115, 110 },
    { "comp", "Comp", 1116, 575, 115, 110 },
    { "hpf", "HPF", 310, 795, 96, 91 },
    { "lpf", "LPF", 474, 795, 96, 91 },
    { "limiter", "Limiter", 628, 795, 96, 91 },
    { "width", "Width", 967, 795, 96, 91 },
    { "mix", "Mix", 1127, 795, 96, 91 }
}};

enum ControlIndex : std::size_t
{
    driveControl,
    vinylControl,
    destroyControl,
    wowControl,
    flutterControl,
    noiseControl,
    dustControl,
    crushControl,
    compControl,
    hpfControl,
    lpfControl,
    limiterControl,
    widthControl,
    mixControl
};
}

MakosizerXAudioProcessorEditor::MakosizerXAudioProcessorEditor (
    MakosizerXAudioProcessor& processor)
    : juce::AudioProcessorEditor (processor),
      audioProcessor (processor)
{
    backgroundImage = juce::ImageCache::getFromMemory (
        BinaryData::makosizerx_background_png,
        BinaryData::makosizerx_background_pngSize);

    for (std::size_t i = 0; i < controlPlacements.size(); ++i)
    {
        const auto& placement = controlPlacements[i];
        auto& slider = sliders[i];

        slider.setName (placement.label);
        slider.setTooltip (placement.label);
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                                    juce::MathConstants<float>::pi * 2.8f, true);
        slider.setLookAndFeel (&rotaryLookAndFeel);
        slider.setOpaque (false);
        slider.setBounds (placement.x, placement.y, placement.width, placement.height);
        addAndMakeVisible (slider);
        slider.addListener (this);

        attachments[i] = std::make_unique<SliderAttachment> (
            processor.getParameterState(), placement.parameterId, slider);
    }

    // LOCKED overlays: change these bounds only with an explicit artwork/layout revision.
    lcdDisplay.setBounds (973, 153, 233, 122);
    addAndMakeVisible (lcdDisplay);
    ledMeter.setBounds (1287, 117, 178, 510);
    addAndMakeVisible (ledMeter);
    // LOCKED: Match the PNG's native dimensions to preserve its 3:2 aspect ratio.
    setSize (1536, 1024);
    updateLCDDisplay();
    startTimerHz (30);
}

MakosizerXAudioProcessorEditor::~MakosizerXAudioProcessorEditor()
{
    stopTimer();

    for (auto& slider : sliders)
    {
        slider.removeListener (this);
        slider.setLookAndFeel (nullptr);
    }
}

void MakosizerXAudioProcessorEditor::paint (juce::Graphics& graphics)
{
    if (backgroundImage.isValid())
        graphics.drawImage (backgroundImage, getLocalBounds().toFloat());
    else
        graphics.fillAll (juce::Colour (0xff202020));
}

void MakosizerXAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    juce::ignoreUnused (slider);
    updateLCDDisplay();
}

void MakosizerXAudioProcessorEditor::timerCallback()
{
    const auto levels = audioProcessor.getAndResetOutputLevels();
    ledMeter.setLevels (levels.left, levels.right);
    outputLevel = juce::jmax (levels.left, levels.right);
    updateLCDDisplay();
}

void MakosizerXAudioProcessorEditor::updateLCDDisplay()
{
    LCDDisplay::State state;
    state.presetName = audioProcessor.getPresetManager().getCurrentPresetName();
    if (state.presetName.isEmpty())
        state.presetName = "INIT";
    state.drive = static_cast<float> (sliders[driveControl].getValue());
    state.crush = static_cast<float> (sliders[crushControl].getValue());
    state.noise = static_cast<float> (sliders[noiseControl].getValue());
    state.wow = static_cast<float> (sliders[wowControl].getValue());
    state.flutter = static_cast<float> (sliders[flutterControl].getValue());
    state.outputLevel = outputLevel;
    lcdDisplay.setState (state);
}
