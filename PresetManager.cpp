#include "PresetManager.h"

#include <array>
#include <cmath>
#include <memory>

namespace
{
constexpr std::size_t parameterCount = 14;

constexpr std::array<const char*, parameterCount> parameterIds {{
    "drive", "vinyl", "destroy", "wow", "flutter", "noise", "dust",
    "crush", "comp", "hpf", "lpf", "limiter", "width", "mix"
}};

struct FactoryPreset
{
    const char* name;
    const char* description;
    std::array<float, parameterCount> values;
};

constexpr std::array<FactoryPreset, 10> factoryPresets {{
    { "Clean", "A restrained, clean starting point.",
      {{ 0.05f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f,
         0.08f, 20.0f, 20000.0f, 0.08f, 1.00f, 1.00f }} },
    { "Master Glue", "Subtle drive and compression for a cohesive master.",
      {{ 0.18f, 0.05f, 0.04f, 0.01f, 0.01f, 0.01f, 0.00f, 0.05f,
         0.38f, 25.0f, 18000.0f, 0.25f, 1.05f, 1.00f }} },
    { "Vinyl Warmth", "Warm vinyl color with gentle motion and surface noise.",
      {{ 0.10f, 0.72f, 0.05f, 0.22f, 0.12f, 0.22f, 0.25f, 0.12f,
         0.18f, 25.0f, 15000.0f, 0.08f, 1.05f, 1.00f }} },
    { "Cassette Melt", "Soft tape wear, modulation, and rounded transients.",
      {{ 0.22f, 0.55f, 0.35f, 0.28f, 0.32f, 0.28f, 0.36f, 0.38f,
         0.28f, 35.0f, 12500.0f, 0.18f, 0.95f, 1.00f }} },
    { "SP303 Vibes", "Sampler-style crunch with a compact, punchy tone.",
      {{ 0.22f, 0.35f, 0.30f, 0.08f, 0.08f, 0.18f, 0.20f, 0.68f,
         0.30f, 35.0f, 14500.0f, 0.14f, 1.00f, 1.00f }} },
    { "Dusty Beat", "A softened beat with pronounced dust and surface texture.",
      {{ 0.12f, 0.22f, 0.10f, 0.12f, 0.08f, 0.42f, 0.80f, 0.24f,
         0.18f, 30.0f, 12500.0f, 0.08f, 1.00f, 1.00f }} },
    { "Radio Tape", "Narrower, noisy tape character with softened highs.",
      {{ 0.28f, 0.40f, 0.32f, 0.28f, 0.18f, 0.60f, 0.50f, 0.35f,
         0.35f, 120.0f, 8000.0f, 0.25f, 0.85f, 1.00f }} },
    { "Crunch Master", "Firm saturation, crushing, and stronger limiting.",
      {{ 0.42f, 0.12f, 0.75f, 0.03f, 0.02f, 0.10f, 0.05f, 0.85f,
         0.55f, 30.0f, 18000.0f, 0.68f, 1.00f, 1.00f }} },
    { "Destroyed", "Heavy degradation with unstable motion and texture.",
      {{ 0.72f, 0.45f, 0.95f, 0.40f, 0.40f, 0.55f, 0.65f, 0.82f,
         0.52f, 70.0f, 10000.0f, 0.65f, 0.75f, 1.00f }} },
    { "Nuclear Destroy", "Maximum damage and aggressive tonal shaping.",
      {{ 1.00f, 0.95f, 1.00f, 0.85f, 0.90f, 1.00f, 1.00f, 1.00f,
         0.85f, 150.0f, 7200.0f, 1.00f, 0.60f, 1.00f }} }
}};

juce::Result validatePresetState (const juce::ValueTree& state,
                                  const juce::Identifier& expectedType)
{
    if (! state.isValid() || state.getType() != expectedType)
        return juce::Result::fail ("Preset file has an invalid parameter state.");

    const juce::Identifier idProperty ("id");
    const juce::Identifier valueProperty ("value");
    for (const auto* parameterId : parameterIds)
    {
        const auto parameter = state.getChildWithProperty (idProperty, juce::var (parameterId));
        if (! parameter.isValid() || ! parameter.hasProperty (valueProperty))
            return juce::Result::fail ("Preset is missing parameter: " + juce::String (parameterId));
    }

    return juce::Result::ok();
}
}

PresetManager::PresetManager (ParameterState& state) noexcept
    : parameterState (state)
{
}

std::vector<PresetManager::FactoryPresetInfo> PresetManager::getFactoryPresets()
{
    std::vector<FactoryPresetInfo> presets;
    presets.reserve (factoryPresets.size());

    for (const auto& preset : factoryPresets)
        presets.push_back ({ preset.name, preset.description });

    return presets;
}

juce::Result PresetManager::loadFactoryPreset (const juce::String& presetName)
{
    const FactoryPreset* selectedPreset = nullptr;
    for (const auto& preset : factoryPresets)
    {
        if (presetName.equalsIgnoreCase (preset.name))
        {
            selectedPreset = &preset;
            break;
        }
    }

    if (selectedPreset == nullptr)
        return juce::Result::fail ("Factory preset not found: " + presetName);

    std::array<juce::RangedAudioParameter*, parameterCount> parameters {};
    for (std::size_t i = 0; i < parameterIds.size(); ++i)
    {
        parameters[i] = parameterState.getParameter (parameterIds[i]);
        if (parameters[i] == nullptr)
            return juce::Result::fail ("Processor is missing parameter: "
                                       + juce::String (parameterIds[i]));

        if (! std::isfinite (selectedPreset->values[i]))
            return juce::Result::fail ("Factory preset contains an invalid value.");
    }

    for (std::size_t i = 0; i < parameterIds.size(); ++i)
    {
        auto* parameter = parameters[i];
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost (
            parameter->convertTo0to1 (selectedPreset->values[i]));
        parameter->endChangeGesture();
    }

    parameterState.state.setProperty ("presetName", selectedPreset->name, nullptr);
    return juce::Result::ok();
}

juce::Result PresetManager::savePreset (const juce::File& file,
                                        const juce::String& presetName)
{
    const auto name = presetName.trim();
    if (name.isEmpty())
        return juce::Result::fail ("Preset name cannot be empty.");

    auto state = parameterState.copyState();
    auto validation = validatePresetState (state, parameterState.state.getType());
    if (validation.failed())
        return validation;

    state.setProperty ("presetName", name, nullptr);
    const auto xml = state.createXml();
    if (xml == nullptr)
        return juce::Result::fail ("Could not serialize preset state.");

    const auto directoryResult = file.getParentDirectory().createDirectory();
    if (directoryResult.failed())
        return directoryResult;

    if (! file.replaceWithText (xml->toString()))
        return juce::Result::fail ("Could not write preset file: " + file.getFullPathName());

    parameterState.state.setProperty ("presetName", name, nullptr);
    return juce::Result::ok();
}

juce::Result PresetManager::loadPreset (const juce::File& file)
{
    if (! file.existsAsFile())
        return juce::Result::fail ("Preset file does not exist: " + file.getFullPathName());

    const auto xml = juce::parseXML (file);
    if (xml == nullptr)
        return juce::Result::fail ("Preset file is not valid XML: " + file.getFullPathName());

    auto state = juce::ValueTree::fromXml (*xml);
    const auto validation = validatePresetState (state, parameterState.state.getType());
    if (validation.failed())
        return validation;

    parameterState.replaceState (state);
    return juce::Result::ok();
}

juce::String PresetManager::getCurrentPresetName() const
{
    return parameterState.state.getProperty ("presetName").toString();
}
