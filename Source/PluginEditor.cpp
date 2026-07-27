#include "PluginEditor.h"

namespace {
    constexpr int editorWidth = 900;
    constexpr int editorHeight = 600;
    constexpr int timerHz = 30;
}

DIRescueAudioProcessorEditor::DIRescueAudioProcessorEditor(
    DIRescueAudioProcessor& processorToUse)
    : AudioProcessorEditor(&processorToUse),
      audioProcessor(processorToUse)
{
    instrumentSelector.setSelectedIndex(0);
    instrumentSelector.onSelectionChanged = [this](int index)
    {
        if (auto* param = audioProcessor.parameters.getParameter("instrument"))
            param->setValueNotifyingHost(static_cast<float>(index));
    };
    addAndMakeVisible(instrumentSelector);

    restoreSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    restoreSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 24);
    restoreSlider.setRange(0.0, 100.0, 0.1);
    restoreSlider.setNumDecimalPlacesToDisplay(0);
    restoreSlider.setTextValueSuffix(" %");
    restoreSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffd8d1c4));
    restoreSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    restoreSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    restoreSlider.setLookAndFeel(&diLookAndFeel);
    addAndMakeVisible(restoreSlider);

    bypassButton.setLookAndFeel(&diLookAndFeel);
    addAndMakeVisible(bypassButton);

    inMeter.setNumSegments(14);
    outMeter.setNumSegments(14);
    addAndMakeVisible(inMeter);
    addAndMakeVisible(outMeter);
    addAndMakeVisible(inClipLed);
    addAndMakeVisible(outClipLed);

    restoreAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.parameters, "restore", restoreSlider);
    bypassAttachment = std::make_unique<ButtonAttachment>(
        audioProcessor.parameters, "bypass", bypassButton);

    audioProcessor.parameters.addParameterListener("instrument", this);
    parameterChanged("instrument", 0.0f);

    setSize(editorWidth, editorHeight);
    setResizable(false, false);
    startTimerHz(timerHz);
}

DIRescueAudioProcessorEditor::~DIRescueAudioProcessorEditor()
{
    audioProcessor.parameters.removeParameterListener("instrument", this);
    stopTimer();
    restoreSlider.setLookAndFeel(nullptr);
    bypassButton.setLookAndFeel(nullptr);
}

void DIRescueAudioProcessorEditor::parameterChanged(const juce::String& parameterID, float)
{
    if (parameterID == "instrument")
        if (auto* v = audioProcessor.parameters.getRawParameterValue("instrument"))
            instrumentSelector.setSelectedIndex(static_cast<int>(v->load()));
}

void DIRescueAudioProcessorEditor::timerCallback()
{
    const float inPeak = audioProcessor.inputPeak.load(std::memory_order_relaxed);
    const float outPeak = audioProcessor.outputPeak.load(std::memory_order_relaxed);

    inMeter.setLevelDb(juce::Decibels::gainToDecibels(inPeak, -60.0f));
    outMeter.setLevelDb(juce::Decibels::gainToDecibels(outPeak, -60.0f));

    inClipLed.refresh(inPeak >= 1.0f);
    outClipLed.refresh(outPeak >= 1.0f);
}

void DIRescueAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff050606));

    auto panel = getLocalBounds().reduced(22).toFloat();
    g.setColour(juce::Colour(0xff171819));
    g.fillRoundedRectangle(panel, 12.0f);
    g.setColour(juce::Colour(0x20000000));
    g.drawRoundedRectangle(panel, 12.0f, 1.0f);

    g.setColour(juce::Colour(0xffd8d1c4));
    g.setFont(juce::FontOptions(36.0f, juce::Font::bold));
    g.drawText("DI RESCUE", 52, 42, 300, 48, juce::Justification::centredLeft);

    g.setFont(juce::FontOptions(13.0f));
    g.setColour(juce::Colour(0xffa0a0a0));
    g.drawText("DI CONDITIONER FOR NAM", 52, 86, 300, 20, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xffd7903f));
    g.drawText("v0.1 OPEN SOURCE", getWidth() - 210, 58, 160, 22, juce::Justification::centredRight);

    g.setColour(juce::Colour(0xffd8d1c4));
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.drawText("INSTRUMENT", 70, 140, 150, 22, juce::Justification::centred);

    g.setFont(juce::FontOptions(12.0f));
    g.drawText("IN", 72, 270, 30, 18, juce::Justification::centred);
    g.drawText("OUT", 132, 270, 30, 18, juce::Justification::centred);
    g.drawText("BYPASS", getWidth() - 180, 225, 100, 22, juce::Justification::centred);

    g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    g.drawText("RESTORE", getWidth() / 2 - 80, 465, 160, 28, juce::Justification::centred);

    g.setColour(juce::Colour(0xffd7903f));
    g.setFont(juce::FontOptions(12.0f));
    g.drawText("PLACE BEFORE NAM", getWidth() / 2 - 140, getHeight() - 62, 280, 24, juce::Justification::centred);

    if (audioProcessor.getLatencySamples() == 0)
    {
        g.setColour(juce::Colour(0xffa0a0a0));
        g.drawText("ZERO LATENCY", getWidth() - 210, getHeight() - 62, 160, 24, juce::Justification::centredRight);
    }
}

void DIRescueAudioProcessorEditor::resized()
{
    instrumentSelector.setBounds(60, 165, 150, 90);

    inClipLed.setBounds(75, 292, 20, 20);
    outClipLed.setBounds(135, 292, 20, 20);

    inMeter.setBounds(72, 320, 26, 150);
    outMeter.setBounds(132, 320, 26, 150);

    restoreSlider.setBounds(getWidth() / 2 - 150, 120, 300, 300);

    bypassButton.setBounds(getWidth() - 180, 250, 100, 40);
}

