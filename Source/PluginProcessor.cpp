#include "PluginProcessor.h"
#include "PluginEditor.h"

DIRescueAudioProcessor::DIRescueAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::mono(), true)
                         .withOutput("Output", juce::AudioChannelSet::mono(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    setLatencySamples(0);
}

void DIRescueAudioProcessor::prepareToPlay(double sampleRate, int)
{
    inputPeak.store(0.0f);
    outputPeak.store(0.0f);

    const auto numChannels = getTotalNumInputChannels();
    channelDsp.clear();
    channelDsp.reserve(static_cast<size_t>(numChannels));
    for (int i = 0; i < numChannels; ++i)
    {
        channelDsp.emplace_back();
        channelDsp.back().prepare(sampleRate);
    }
}

void DIRescueAudioProcessor::reset()
{
    for (auto& channel : channelDsp)
        channel.reset();
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

    auto* rawInstrument = parameters.getRawParameterValue("instrument");
    auto* rawRestore = parameters.getRawParameterValue("restore");
    auto* rawBypass = parameters.getRawParameterValue("bypass");
    jassert(rawInstrument != nullptr && rawRestore != nullptr && rawBypass != nullptr);

    const int instrumentIndex = rawInstrument != nullptr
        ? static_cast<int>(rawInstrument->load())
        : 0;

    // APVTS returns the parameter's denormalised value. Restore is declared
    // as 0..100, so multiplying by 100 here would compress the complete DSP
    // range into the first 1% of the user control.
    const float restore = rawRestore != nullptr ? rawRestore->load() : 0.0f;
    const bool bypass = rawBypass != nullptr && rawBypass->load() > 0.5f;

    for (auto channel = 0; channel < getTotalNumInputChannels(); ++channel)
    {
        const auto idx = static_cast<size_t>(channel);
        channelDsp[idx].setInstrument(instrumentIndex);
        channelDsp[idx].setTargets(restore, bypass);
    }

    for (auto channel = 0; channel < getTotalNumInputChannels(); ++channel)
    {
        const auto idx = static_cast<size_t>(channel);
        auto* channelData = buffer.getWritePointer(channel);
        for (auto s = 0; s < buffer.getNumSamples(); ++s)
            channelData[s] = channelDsp[idx].processSample(channelData[s]);
    }

    float inPeak = 0.0f;
    float outPeak = 0.0f;

    for (auto& channel : channelDsp)
    {
        inPeak = std::max(inPeak, channel.getAndResetInputPeak());
        outPeak = std::max(outPeak, channel.getAndResetOutputPeak());
    }

    auto updateAtomicPeak = [](std::atomic<float>& peak, float local)
    {
        float expected = peak.load(std::memory_order_acquire);

        while (local > expected)
        {
            if (peak.compare_exchange_weak(expected, local,
                                           std::memory_order_acq_rel,
                                           std::memory_order_acquire))
                break;
        }
    };

    updateAtomicPeak(inputPeak, inPeak);
    updateAtomicPeak(outputPeak, outPeak);
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
    {
        auto tree = juce::ValueTree::fromXml(*xml);
        if (tree.isValid() && tree.getType() == parameters.state.getType())
            parameters.replaceState(tree);
    }
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
