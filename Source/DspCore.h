#pragma once

#include <JuceHeader.h>

#include <array>

namespace rescue_dsp {

class ChannelDsp
{
public:
    ChannelDsp() = default;

    void prepare(double newSampleRate);
    void reset() noexcept;

    void setInstrument(int index) noexcept;
    void setTargets(float restore0To100, bool bypass) noexcept;

    float processSample(float input) noexcept;

    float getAndResetInputPeak() noexcept;
    float getAndResetOutputPeak() noexcept;

private:
    void updateEnvelope(float& env, float target, float attack, float release) const noexcept;
    void resetFilters() noexcept;
    void buildCoefficients();

    struct FilterCoeffs
    {
        juce::dsp::IIR::Coefficients<float>::Ptr safety;
        juce::dsp::IIR::Coefficients<float>::Ptr body;
        juce::dsp::IIR::Coefficients<float>::Ptr presence;
        juce::dsp::IIR::Coefficients<float>::Ptr top;
        juce::dsp::IIR::Coefficients<float>::Ptr shelf;
        juce::dsp::IIR::Coefficients<float>::Ptr harmHp;
        juce::dsp::IIR::Coefficients<float>::Ptr harmLp;
    };

    double sampleRate = 44100.0;
    int currentInstrument = 0;

    std::array<FilterCoeffs, 2> filterCoeffs;

    float restoreNorm = 0.0f;
    float targetRestoreNorm = 0.0f;
    float bypassMix = 1.0f;
    float targetBypassMix = 1.0f;

    juce::dsp::IIR::Filter<float> safetyFilter;

    juce::dsp::IIR::Filter<float> bodyFilter;
    juce::dsp::IIR::Filter<float> presenceFilter;
    juce::dsp::IIR::Filter<float> topFilter;
    juce::dsp::IIR::Filter<float> shelfFilter;

    juce::dsp::IIR::Filter<float> harmonicHp;
    juce::dsp::IIR::Filter<float> harmonicLp;

    float bodyEnv = 0.0f, presEnv = 0.0f, topEnv = 0.0f, signalEnv = 0.0f;
    float correction = 0.0f;

    float fastEnv = 0.0f, slowEnv = 0.0f;

    float restoreCoef = 0.0f, bypassCoef = 0.0f;
    float maxTransLinear = 0.0f;
    float bodyAttackCoef = 0.0f, bodyReleaseCoef = 0.0f;
    float presAttackCoef = 0.0f, presReleaseCoef = 0.0f;
    float topAttackCoef = 0.0f, topReleaseCoef = 0.0f;
    float signalAttackCoef = 0.0f, signalReleaseCoef = 0.0f;
    float spectralAttackCoef = 0.0f, spectralReleaseCoef = 0.0f;
    float fastAttackCoef = 0.0f, fastReleaseCoef = 0.0f;
    float slowAttackCoef = 0.0f, slowReleaseCoef = 0.0f;

    float inputPeak = 0.0f, outputPeak = 0.0f;

    static constexpr float maxSpectralDb = 2.5f;
    static constexpr float maxTransientDb = 0.8f;
    static constexpr float levelCompDb = 1.8f;
    static constexpr float branchDrive = 4.0f;
    static constexpr float harmonicLevel = 0.04f;
};

} // namespace rescue_dsp
