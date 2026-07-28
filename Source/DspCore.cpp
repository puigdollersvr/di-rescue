#include "DspCore.h"

#include <algorithm>
#include <cmath>

namespace rescue_dsp {

namespace {

inline float safeValue(float x) noexcept
{
    if (! std::isfinite(x))
        return 0.0f;

    if (x > -1.0e-36f && x < 1.0e-36f)
        return 0.0f;

    return x;
}

inline float envCoef(float seconds, double sampleRate) noexcept
{
    const float t = std::max(0.0001f, seconds);
    return 1.0f - std::exp(-1.0f / (t * static_cast<float>(sampleRate)));
}

inline float smoothStep(float x) noexcept
{
    x = std::clamp(x, 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

inline float powerToDecibels(float power) noexcept
{
    return 10.0f * std::log10(std::max(power, 1.0e-12f));
}

inline float stableLogCosh(float x) noexcept
{
    const float ax = std::abs(x);
    return ax + std::log1p(std::exp(-2.0f * ax)) - std::log(2.0f);
}

struct Band
{
    float lo = 0.0f;
    float hi = 0.0f;
};

struct InstrumentConfig
{
    Band body;
    Band presence;
    Band top;
    float shelfFreq = 2000.0f;
    float harmHp = 1500.0f;
    float harmLp = 5000.0f;
    float harmPostLp = 7000.0f;
    float brightnessReferenceDb = -9.0f;
    float brightnessRangeDb = 12.0f;
    float fastAttack = 0.001f;
    float fastRelease = 0.030f;
    float slowAttack = 0.010f;
    float slowRelease = 0.100f;
};

const InstrumentConfig guitarConfig {
    { 120.0f,  800.0f },
    { 1500.0f, 4500.0f },
    { 4500.0f, 8000.0f },
    2000.0f,
    1500.0f,
    5000.0f,
    7000.0f,
    -9.0f,
    12.0f,
    0.001f,
    0.030f,
    0.010f,
    0.100f
};

const InstrumentConfig bassConfig {
    { 60.0f,   400.0f },
    { 700.0f,  2000.0f },
    { 2000.0f, 5000.0f },
    1000.0f,
    800.0f,
    3000.0f,
    5000.0f,
    -12.0f,
    14.0f,
    0.0015f,
    0.040f,
    0.020f,
    0.150f
};

inline const InstrumentConfig& getConfig(int instrument) noexcept
{
    return (instrument == 1) ? bassConfig : guitarConfig;
}

inline float bandCentre(float lo, float hi) noexcept
{
    return std::sqrt(lo * hi);
}

inline float bandQ(float lo, float hi) noexcept
{
    const float centre = bandCentre(lo, hi);
    const float width = std::max(1.0f, hi - lo);
    return centre / width;
}

inline float safeFrequency(float requested, double sampleRate) noexcept
{
    return std::min(requested, 0.45f * static_cast<float>(sampleRate));
}

inline float octaveWidth(const Band& band) noexcept
{
    return std::max(0.25f, std::log2(band.hi / band.lo));
}

} // namespace

void ChannelDsp::prepare(double newSampleRate)
{
    sampleRate = std::max(1.0, newSampleRate);
    buildCoefficients();

    safetyFilter.coefficients = filterCoeffs[0].safety;
    currentInstrument = -1;
    pendingInstrument = 0;
    applyInstrument(0);
    reset();
}

void ChannelDsp::buildCoefficients()
{
    for (int i = 0; i < 2; ++i)
    {
        const auto& cfg = getConfig(i);
        auto& c = filterCoeffs[static_cast<size_t>(i)];

        c.safety = juce::dsp::IIR::Coefficients<float>::makeHighPass(
            sampleRate, safeFrequency(20.0f, sampleRate), 0.707f);
        c.body = juce::dsp::IIR::Coefficients<float>::makeBandPass(
            sampleRate,
            safeFrequency(bandCentre(cfg.body.lo, cfg.body.hi), sampleRate),
            bandQ(cfg.body.lo, cfg.body.hi));
        c.presence = juce::dsp::IIR::Coefficients<float>::makeBandPass(
            sampleRate,
            safeFrequency(bandCentre(cfg.presence.lo, cfg.presence.hi), sampleRate),
            bandQ(cfg.presence.lo, cfg.presence.hi));
        c.top = juce::dsp::IIR::Coefficients<float>::makeBandPass(
            sampleRate,
            safeFrequency(bandCentre(cfg.top.lo, cfg.top.hi), sampleRate),
            bandQ(cfg.top.lo, cfg.top.hi));
        c.naturalShelf = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate, safeFrequency(cfg.shelfFreq, sampleRate), 0.707f,
            juce::Decibels::decibelsToGain(naturalSpectralDb));
        c.auditionShelf = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate, safeFrequency(cfg.shelfFreq, sampleRate), 0.707f,
            juce::Decibels::decibelsToGain(auditionSpectralDb));
        c.harmHp = juce::dsp::IIR::Coefficients<float>::makeHighPass(
            sampleRate, safeFrequency(cfg.harmHp, sampleRate), 0.707f);
        c.harmLp = juce::dsp::IIR::Coefficients<float>::makeLowPass(
            sampleRate, safeFrequency(cfg.harmLp, sampleRate), 0.707f);
        c.harmPostLp = juce::dsp::IIR::Coefficients<float>::makeLowPass(
            sampleRate, safeFrequency(cfg.harmPostLp, sampleRate), 0.707f);
    }
}

void ChannelDsp::resetInstrumentState() noexcept
{
    bodyFilter.reset();
    presenceFilter.reset();
    topFilter.reset();
    naturalShelfFilter.reset();
    auditionShelfFilter.reset();
    harmonicHp.reset();
    harmonicLp.reset();
    harmonicPostLp.reset();

    bodyEnv = presEnv = topEnv = signalEnv = 0.0f;
    correction = 0.0f;
    fastEnv = slowEnv = 0.0f;
    previousHarmonicInput = 0.0f;
}

void ChannelDsp::reset() noexcept
{
    safetyFilter.reset();
    resetInstrumentState();

    restoreNorm = targetRestoreNorm = 0.0f;
    bypassMix = targetBypassMix = 1.0f;
    instrumentMix = targetInstrumentMix = 1.0f;

    inputPeak = outputPeak = 0.0f;
}

void ChannelDsp::applyInstrument(int index) noexcept
{
    currentInstrument = (index == 1) ? 1 : 0;
    pendingInstrument = currentInstrument;

    const auto& cfg = getConfig(currentInstrument);
    const auto& c = filterCoeffs[static_cast<size_t>(currentInstrument)];

    bodyFilter.coefficients = c.body;
    presenceFilter.coefficients = c.presence;
    topFilter.coefficients = c.top;
    naturalShelfFilter.coefficients = c.naturalShelf;
    auditionShelfFilter.coefficients = c.auditionShelf;
    harmonicHp.coefficients = c.harmHp;
    harmonicLp.coefficients = c.harmLp;
    harmonicPostLp.coefficients = c.harmPostLp;

    restoreCoef = envCoef(0.010f, sampleRate);
    bypassCoef = envCoef(0.005f, sampleRate);
    instrumentCoef = envCoef(0.005f, sampleRate);

    bodyAttackCoef = envCoef(0.010f, sampleRate);
    bodyReleaseCoef = envCoef(0.150f, sampleRate);
    presAttackCoef = envCoef(0.010f, sampleRate);
    presReleaseCoef = envCoef(0.120f, sampleRate);
    topAttackCoef = envCoef(0.010f, sampleRate);
    topReleaseCoef = envCoef(0.120f, sampleRate);
    signalAttackCoef = envCoef(0.003f, sampleRate);
    signalReleaseCoef = envCoef(0.200f, sampleRate);

    spectralAttackCoef = envCoef(0.150f, sampleRate);
    spectralReleaseCoef = envCoef(0.500f, sampleRate);

    fastAttackCoef = envCoef(cfg.fastAttack, sampleRate);
    fastReleaseCoef = envCoef(cfg.fastRelease, sampleRate);
    slowAttackCoef = envCoef(cfg.slowAttack, sampleRate);
    slowReleaseCoef = envCoef(cfg.slowRelease, sampleRate);

    resetInstrumentState();
}

void ChannelDsp::setInstrument(int index) noexcept
{
    pendingInstrument = (index == 1) ? 1 : 0;
}

void ChannelDsp::setTargets(float restore0To100, bool bypass) noexcept
{
    targetRestoreNorm = std::clamp(restore0To100 / 100.0f, 0.0f, 1.0f);
    targetBypassMix = bypass ? 0.0f : 1.0f;
}

void ChannelDsp::updateEnvelope(float& env, float target,
                                float attack, float release) const noexcept
{
    const float coef = (target > env) ? attack : release;
    env += (target - env) * coef;
}

float ChannelDsp::processHarmonicResidual(float input) noexcept
{
    const float midpoint = 0.5f * (input + previousHarmonicInput);
    const float delta = input - previousHarmonicInput;

    float shaped = 0.0f;
    if (std::abs(delta) > 1.0e-5f)
    {
        const auto antiderivative = [](float x) noexcept
        {
            return stableLogCosh(branchDrive * x)
                 / (branchDrive * branchDrive);
        };

        shaped = (antiderivative(input) - antiderivative(previousHarmonicInput))
               / delta;
    }
    else
    {
        shaped = std::tanh(branchDrive * midpoint) / branchDrive;
    }

    previousHarmonicInput = input;

    // Subtract the ADAA response of the identity function. This removes the
    // linear leakage and leaves only the nonlinear residual.
    return safeValue(shaped - midpoint);
}

float ChannelDsp::processSample(float in) noexcept
{
    in = safeValue(in);
    inputPeak = std::max(inputPeak, std::abs(in));

    restoreNorm += (targetRestoreNorm - restoreNorm) * restoreCoef;
    bypassMix += (targetBypassMix - bypassMix) * bypassCoef;

    if (pendingInstrument != currentInstrument)
        targetInstrumentMix = 0.0f;
    else
        targetInstrumentMix = 1.0f;

    instrumentMix += (targetInstrumentMix - instrumentMix) * instrumentCoef;

    if (pendingInstrument != currentInstrument && instrumentMix < 1.0e-3f)
    {
        applyInstrument(pendingInstrument);
        instrumentMix = 0.0f;
        targetInstrumentMix = 1.0f;
    }

    // 1. Input safety: DC blocker + subsonic cut.
    const float dry = safetyFilter.processSample(in);

    // 2. Power detection. Energy density is approximately normalised by the
    // octave width of each analysis band.
    const auto& cfg = getConfig(currentInstrument);
    const float body = bodyFilter.processSample(dry);
    const float pres = presenceFilter.processSample(dry);
    const float top = topFilter.processSample(dry);

    updateEnvelope(bodyEnv, body * body, bodyAttackCoef, bodyReleaseCoef);
    updateEnvelope(presEnv, pres * pres, presAttackCoef, presReleaseCoef);
    updateEnvelope(topEnv, top * top, topAttackCoef, topReleaseCoef);
    updateEnvelope(signalEnv, dry * dry, signalAttackCoef, signalReleaseCoef);

    constexpr float silencePower = 1.0e-7f; // -70 dBFS
    float correctionTarget = 0.0f;

    if (signalEnv > silencePower)
    {
        const float bodyDensity = bodyEnv / octaveWidth(cfg.body);
        const float presenceDensity = presEnv / octaveWidth(cfg.presence);
        const float topDensity = topEnv / octaveWidth(cfg.top);
        const float highDensity = 0.65f * presenceDensity
                                + 0.35f * topDensity;

        const float brightnessDb = powerToDecibels(
            (highDensity + 1.0e-12f) / (bodyDensity + 1.0e-12f));
        const float deficit = (cfg.brightnessReferenceDb - brightnessDb)
                            / cfg.brightnessRangeDb;
        correctionTarget = smoothStep(deficit);
    }

    updateEnvelope(correction, correctionTarget,
                   spectralAttackCoef, spectralReleaseCoef);

    // Restore 0..65 stays conservative and detector-led. The upper part of
    // the control becomes an explicit audition zone; at 100 the difference
    // remains clearly audible even when the detector considers the DI bright.
    const float upper = smoothStep((restoreNorm - 0.65f) / 0.35f);

    const float naturalShelf = naturalShelfFilter.processSample(dry);
    const float auditionShelf = auditionShelfFilter.processSample(dry);
    const float detectorLedAmount = restoreNorm * correction;
    float x = dry + (naturalShelf - dry) * detectorLedAmount;
    x += (auditionShelf - x) * upper;

    // 3. Relative short/long-term power, expressed in dB.
    updateEnvelope(fastEnv, x * x, fastAttackCoef, fastReleaseCoef);
    updateEnvelope(slowEnv, x * x, slowAttackCoef, slowReleaseCoef);

    const float relativeTransientDb = powerToDecibels(
        (fastEnv + 1.0e-12f) / (slowEnv + 1.0e-12f));
    const float transientAmount = smoothStep(
        (relativeTransientDb - 1.0f) / 9.0f);
    const float transientDb = (naturalTransientDb * restoreNorm
                            + auditionTransientDb * upper)
                            * transientAmount;
    x *= juce::Decibels::decibelsToGain(transientDb);

    // 4. Prefiltered, first-order ADAA tanh residual. ADAA reduces aliasing
    // without changing the zero-latency contract of the prototype.
    float branch = harmonicHp.processSample(dry);
    branch = harmonicLp.processSample(branch);
    branch = processHarmonicResidual(branch);
    branch = harmonicPostLp.processSample(branch);

    const float harmonicAmount = 0.30f * restoreNorm * correction
                               + 0.20f * upper;
    x += branch * harmonicAmount;

    // 5. Small deterministic compensation. Avoid the previous -3 dB global
    // attenuation; exact coefficients remain subject to corpus validation.
    const float compDb = -0.25f * detectorLedAmount - 0.50f * upper;
    x *= juce::Decibels::decibelsToGain(compDb);

    // Fade processing depth to dry before changing instrument filter banks.
    x = dry + (x - dry) * instrumentMix;

    // 6. Raw-input bypass with a short crossfade.
    x = in + (x - in) * bypassMix;

    x = safeValue(x);
    outputPeak = std::max(outputPeak, std::abs(x));
    return x;
}

float ChannelDsp::getAndResetInputPeak() noexcept
{
    const float p = inputPeak;
    inputPeak = 0.0f;
    return p;
}

float ChannelDsp::getAndResetOutputPeak() noexcept
{
    const float p = outputPeak;
    outputPeak = 0.0f;
    return p;
}

} // namespace rescue_dsp
