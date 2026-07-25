#include "PluginEditor.h"

DIRescueAudioProcessorEditor::DIRescueAudioProcessorEditor(
    DIRescueAudioProcessor& processorToUse)
    : AudioProcessorEditor(&processorToUse),
      processor(processorToUse)
{
    instrumentSelector.addItem("GUITAR", 1);
    instrumentSelector.addItem("BASS", 2);
    addAndMakeVisible(instrumentSelector);

    restoreSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    restoreSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 28);
    restoreSlider.setTextValueSuffix(" %");
    addAndMakeVisible(restoreSlider);

    addAndMakeVisible(bypassButton);

    instrumentAttachment = std::make_unique<ComboAttachment>(
        processor.parameters, "instrument", instrumentSelector);
    restoreAttachment = std::make_unique<SliderAttachment>(
        processor.parameters, "restore", restoreSlider);
    bypassAttachment = std::make_unique<ButtonAttachment>(
        processor.parameters, "bypass", bypassButton);

    setSize(900, 600);
}

void DIRescueAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    graphics.fillAll(juce::Colour(0xff050606));

    auto panel = getLocalBounds().reduced(22).toFloat();
    graphics.setColour(juce::Colour(0xff171819));
    graphics.fillRoundedRectangle(panel, 12.0f);

    graphics.setColour(juce::Colour(0xffd8d1c4));
    graphics.setFont(juce::FontOptions(28.0f, juce::Font::bold));
    graphics.drawText("DI RESCUE", 52, 38, 300, 42, juce::Justification::centredLeft);

    graphics.setFont(juce::FontOptions(13.0f));
    graphics.drawText("DI CONDITIONER FOR NAM",
                      54, 76, 300, 24, juce::Justification::centredLeft);

    graphics.setColour(juce::Colour(0xffd7903f));
    graphics.drawText("PROTOTYPE",
                      getWidth() - 190, 48, 130, 24, juce::Justification::centredRight);

    graphics.setColour(juce::Colour(0xffd8d1c4));
    graphics.drawText("RESTORE",
                      getWidth() / 2 - 80, 470, 160, 28, juce::Justification::centred);
    graphics.drawText("PLACE BEFORE NAM",
                      getWidth() / 2 - 120, getHeight() - 62, 240, 24,
                      juce::Justification::centred);
}

void DIRescueAudioProcessorEditor::resized()
{
    instrumentSelector.setBounds(72, 230, 150, 34);
    restoreSlider.setBounds(getWidth() / 2 - 135, 175, 270, 290);
    bypassButton.setBounds(getWidth() - 210, 260, 130, 36);
}

