#pragma once

#include <juce_dsp/juce_dsp.h>

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
    void resetInstrumentState() noexcept;
    void applyInstrument(int index) noexcept;
    void buildCoefficients();
    float processHarmonicResidual(float input) noexcept;

    struct FilterCoeffs
    {
        juce::dsp::IIR::Coefficients<float>::Ptr safety;
        juce::dsp::IIR::Coefficients<float>::Ptr body;
        juce::dsp::IIR::Coefficients<float>::Ptr presence;
        juce::dsp::IIR::Coefficients<float>::Ptr top;
        juce::dsp::IIR::Coefficients<float>::Ptr naturalShelf;
        juce::dsp::IIR::Coefficients<float>::Ptr auditionShelf;
        juce::dsp::IIR::Coefficients<float>::Ptr harmHp;
        juce::dsp::IIR::Coefficients<float>::Ptr harmLp;
        juce::dsp::IIR::Coefficients<float>::Ptr harmPostLp;
    };

    double sampleRate = 44100.0;
    int currentInstrument = -1;
    int pendingInstrument = 0;

    std::array<FilterCoeffs, 2> filterCoeffs;

    float restoreNorm = 0.0f;
    float targetRestoreNorm = 0.0f;
    float bypassMix = 1.0f;
    float targetBypassMix = 1.0f;
    float instrumentMix = 1.0f;
    float targetInstrumentMix = 1.0f;

    juce::dsp::IIR::Filter<float> safetyFilter;

    juce::dsp::IIR::Filter<float> bodyFilter;
    juce::dsp::IIR::Filter<float> presenceFilter;
    juce::dsp::IIR::Filter<float> topFilter;
    juce::dsp::IIR::Filter<float> naturalShelfFilter;
    juce::dsp::IIR::Filter<float> auditionShelfFilter;

    juce::dsp::IIR::Filter<float> harmonicHp;
    juce::dsp::IIR::Filter<float> harmonicLp;
    juce::dsp::IIR::Filter<float> harmonicPostLp;

    float bodyEnv = 0.0f;
    float presEnv = 0.0f;
    float topEnv = 0.0f;
    float signalEnv = 0.0f;
    float correction = 0.0f;

    float fastEnv = 0.0f;
    float slowEnv = 0.0f;
    float previousHarmonicInput = 0.0f;

    float restoreCoef = 0.0f;
    float bypassCoef = 0.0f;
    float instrumentCoef = 0.0f;
    float bodyAttackCoef = 0.0f;
    float bodyReleaseCoef = 0.0f;
    float presAttackCoef = 0.0f;
    float presReleaseCoef = 0.0f;
    float topAttackCoef = 0.0f;
    float topReleaseCoef = 0.0f;
    float signalAttackCoef = 0.0f;
    float signalReleaseCoef = 0.0f;
    float spectralAttackCoef = 0.0f;
    float spectralReleaseCoef = 0.0f;
    float fastAttackCoef = 0.0f;
    float fastReleaseCoef = 0.0f;
    float slowAttackCoef = 0.0f;
    float slowReleaseCoef = 0.0f;

    float inputPeak = 0.0f;
    float outputPeak = 0.0f;

    static constexpr float naturalSpectralDb = 2.5f;
    static constexpr float auditionSpectralDb = 4.5f;
    static constexpr float naturalTransientDb = 1.2f;
    static constexpr float auditionTransientDb = 0.8f;
    static constexpr float branchDrive = 4.0f;
};

} // namespace rescue_dsp
