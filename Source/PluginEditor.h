#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"

class DIRescueAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit DIRescueAudioProcessorEditor(DIRescueAudioProcessor&);
    ~DIRescueAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DIRescueAudioProcessor& processor;

    juce::ComboBox instrumentSelector;
    juce::Slider restoreSlider;
    juce::ToggleButton bypassButton { "BYPASS" };

    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<ComboAttachment> instrumentAttachment;
    std::unique_ptr<SliderAttachment> restoreAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DIRescueAudioProcessorEditor)
};

