#include "PluginProcessor.h"
#include "PluginEditor.h"

DIRescueAudioProcessor::DIRescueAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::mono(), true)
                         .withOutput("Output", juce::AudioChannelSet::mono(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

void DIRescueAudioProcessor::prepareToPlay(double, int)
{
    inputPeak.store(0.0f);
    outputPeak.store(0.0f);
}

void DIRescueAudioProcessor::releaseResources()
{
}

bool DIRescueAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& input = layouts.getMainInputChannelSet();
    const auto& output = layouts.getMainOutputChannelSet();

    if (input != output)
        return false;

    return input == juce::AudioChannelSet::mono()
        || input == juce::AudioChannelSet::stereo();
}

void DIRescueAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (auto channel = getTotalNumInputChannels();
         channel < getTotalNumOutputChannels();
         ++channel)
    {
        buffer.clear(channel, 0, buffer.getNumSamples());
    }

    float peak = 0.0f;

    for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
        peak = juce::jmax(peak, buffer.getMagnitude(channel, 0, buffer.getNumSamples()));

    inputPeak.store(peak, std::memory_order_relaxed);

    // Prototype scaffold: DSP modules are intentionally implemented by the
    // tasks in docs/tasks_prototype.md. Audio currently passes through.

    outputPeak.store(peak, std::memory_order_relaxed);
}

juce::AudioProcessorEditor* DIRescueAudioProcessor::createEditor()
{
    return new DIRescueAudioProcessorEditor(*this);
}

bool DIRescueAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String DIRescueAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DIRescueAudioProcessor::acceptsMidi() const
{
    return false;
}

bool DIRescueAudioProcessor::producesMidi() const
{
    return false;
}

bool DIRescueAudioProcessor::isMidiEffect() const
{
    return false;
}

double DIRescueAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DIRescueAudioProcessor::getNumPrograms()
{
    return 1;
}

int DIRescueAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DIRescueAudioProcessor::setCurrentProgram(int)
{
}

const juce::String DIRescueAudioProcessor::getProgramName(int)
{
    return {};
}

void DIRescueAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void DIRescueAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void DIRescueAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorValueTreeState::ParameterLayout
DIRescueAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> layout;

    layout.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "instrument", 1 },
        "Instrument",
        juce::StringArray { "Guitar", "Bass" },
        0));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "restore", 1 },
        "Restore",
        juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f },
        0.0f));

    layout.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "bypass", 1 },
        "Bypass",
        false));

    return { layout.begin(), layout.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DIRescueAudioProcessor();
}

