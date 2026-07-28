#pragma once

#include <atomic>

#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "UiComponents.h"

class DIRescueAudioProcessorEditor final
    : public juce::AudioProcessorEditor,
      private juce::AudioProcessorValueTreeState::Listener,
      private juce::Timer
{
public:
    explicit DIRescueAudioProcessorEditor(DIRescueAudioProcessor&);
    ~DIRescueAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void timerCallback() override;

    DIRescueAudioProcessor& audioProcessor;

    ui::DiRescueLookAndFeel diLookAndFeel;

    ui::InstrumentSelector instrumentSelector;
    juce::Slider restoreSlider;
    juce::ToggleButton bypassButton;

    ui::SegmentedMeter inMeter;
    ui::SegmentedMeter outMeter;
    ui::ClipLed inClipLed;
    ui::ClipLed outClipLed;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> restoreAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment;

    std::atomic<int> pendingInstrumentIndex { -1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DIRescueAudioProcessorEditor)
};

