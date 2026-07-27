#include "DspCore.h"

#include <algorithm>
#include <cmath>

namespace rescue_dsp {

namespace {

inline float safeValue(float x) noexcept
{
    if (std::isnan(x) || std::isinf(x))
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

} // namespace

void ChannelDsp::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
    buildCoefficients();
    currentInstrument = -1;
    setInstrument(0);
    reset();
}

void ChannelDsp::buildCoefficients()
{
    for (int i = 0; i < 2; ++i)
    {
        const auto& cfg = getConfig(i);
        auto& c = filterCoeffs[i];

        c.safety = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.0f, 0.707f);
        c.body = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, bandCentre(cfg.body.lo, cfg.body.hi), bandQ(cfg.body.lo, cfg.body.hi));
        c.presence = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, bandCentre(cfg.presence.lo, cfg.presence.hi), bandQ(cfg.presence.lo, cfg.presence.hi));
        c.top = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, bandCentre(cfg.top.lo, cfg.top.hi), bandQ(cfg.top.lo, cfg.top.hi));
        c.shelf = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, cfg.shelfFreq, 0.707f, juce::Decibels::decibelsToGain(maxSpectralDb));
        c.harmHp = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, cfg.harmHp, 0.707f);
        c.harmLp = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, cfg.harmLp, 0.707f);
    }
}

void ChannelDsp::resetFilters() noexcept
{
    safetyFilter.reset();
    bodyFilter.reset();
    presenceFilter.reset();
    topFilter.reset();
    shelfFilter.reset();
    harmonicHp.reset();
    harmonicLp.reset();
}

void ChannelDsp::reset() noexcept
{
    resetFilters();

    bodyEnv = presEnv = topEnv = signalEnv = 0.0f;
    correction = 0.0f;
    fastEnv = slowEnv = 0.0f;

    restoreNorm = targetRestoreNorm = 0.0f;
    bypassMix = targetBypassMix = 1.0f;

    inputPeak = outputPeak = 0.0f;
}

void ChannelDsp::setInstrument(int index) noexcept
{
    const int selected = (index == 1) ? 1 : 0;
    if (selected == currentInstrument)
        return;
    currentInstrument = selected;
    const auto& cfg = getConfig(currentInstrument);
    const auto& c = filterCoeffs[currentInstrument];

    safetyFilter.coefficients = c.safety;
    bodyFilter.coefficients = c.body;
    presenceFilter.coefficients = c.presence;
    topFilter.coefficients = c.top;
    shelfFilter.coefficients = c.shelf;
    harmonicHp.coefficients = c.harmHp;
    harmonicLp.coefficients = c.harmLp;

    restoreCoef  = envCoef(0.005f, sampleRate);
    bypassCoef   = envCoef(0.005f, sampleRate);

    bodyAttackCoef   = envCoef(0.005f, sampleRate);
    bodyReleaseCoef  = envCoef(0.100f, sampleRate);
    presAttackCoef   = envCoef(0.002f, sampleRate);
    presReleaseCoef  = envCoef(0.080f, sampleRate);
    topAttackCoef    = envCoef(0.002f, sampleRate);
    topReleaseCoef   = envCoef(0.080f, sampleRate);
    signalAttackCoef = envCoef(0.001f, sampleRate);
    signalReleaseCoef= envCoef(0.150f, sampleRate);

    spectralAttackCoef  = envCoef(0.050f, sampleRate);
    spectralReleaseCoef = envCoef(0.500f, sampleRate);

    fastAttackCoef  = envCoef(cfg.fastAttack,  sampleRate);
    fastReleaseCoef = envCoef(cfg.fastRelease, sampleRate);
    slowAttackCoef  = envCoef(cfg.slowAttack,  sampleRate);
    slowReleaseCoef = envCoef(cfg.slowRelease, sampleRate);

    maxTransLinear = std::pow(10.0f, maxTransientDb / 20.0f) - 1.0f;
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

float ChannelDsp::processSample(float in) noexcept
{
    in = safeValue(in);
    inputPeak = std::max(inputPeak, std::abs(in));

    // 1. Input safety: DC blocker + subsonic cut
    const float dry = safetyFilter.processSample(in);

    // 2. Energy detection per band
    const float body = bodyFilter.processSample(dry);
    const float pres = presenceFilter.processSample(dry);
    const float top  = topFilter.processSample(dry);

    updateEnvelope(bodyEnv, std::abs(body), bodyAttackCoef, bodyReleaseCoef);
    updateEnvelope(presEnv, std::abs(pres), presAttackCoef, presReleaseCoef);
    updateEnvelope(topEnv,  std::abs(top),  topAttackCoef,  topReleaseCoef);
    updateEnvelope(signalEnv, std::abs(dry), signalAttackCoef, signalReleaseCoef);

    // 3. Adaptive spectral correction
    constexpr float silenceThreshold = 0.000316f; // -70 dBFS linear
    float correctionTarget = 0.0f;

    if (signalEnv > silenceThreshold)
    {
        const float high = presEnv + topEnv;
        const float total = bodyEnv + high + 1.0e-10f;
        const float balance = high / total;
        // balance > 0.5 is bright enough; below that start restoring
        correctionTarget = 1.0f - balance * 2.0f;
        correctionTarget = std::clamp(correctionTarget, 0.0f, 1.0f);
    }

    updateEnvelope(correction, correctionTarget,
                   spectralAttackCoef, spectralReleaseCoef);

    restoreNorm += (targetRestoreNorm - restoreNorm) * restoreCoef;
    bypassMix   += (targetBypassMix   - bypassMix)   * bypassCoef;

    const float shelfOut = shelfFilter.processSample(dry);
    float x = dry + (shelfOut - dry) * restoreNorm * correction;

    // 4. Transient restorer
    updateEnvelope(fastEnv, std::abs(x), fastAttackCoef, fastReleaseCoef);
    updateEnvelope(slowEnv, std::abs(x), slowAttackCoef, slowReleaseCoef);

    const float trans = std::max(0.0f, fastEnv - slowEnv);
    const float relativeTrans = trans / (slowEnv + 1.0e-10f);
    const float transNorm = std::clamp(relativeTrans * 0.5f, 0.0f, 1.0f);

    const float transGain = 1.0f + maxTransLinear * restoreNorm * transNorm;
    x *= transGain;

    // 5. Harmonic restorer (parallel branch)
    float branch = harmonicHp.processSample(dry);
    branch = harmonicLp.processSample(branch);
    branch = std::tanh(branchDrive * branch);
    x += branch * harmonicLevel * restoreNorm;

    // 6. Deterministic level compensation
    const float compDb = -levelCompDb * restoreNorm;
    x *= juce::Decibels::decibelsToGain(compDb);

    // 7. Bypass crossfade (dry = raw input, processed = x)
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
