#include "DspCore.h"

#include <cmath>
#include <iostream>
#include <random>

using namespace rescue_dsp;

static bool finite(float x)
{
    return std::isfinite(x);
}

static int test_silence()
{
    ChannelDsp dsp;
    dsp.prepare(44100.0);
    dsp.setInstrument(0);
    dsp.setTargets(0.0f, false);

    for (int i = 0; i < 44100; ++i)
    {
        float out = dsp.processSample(0.0f);
        if (!finite(out) || std::abs(out) > 1e-6f)
        {
            std::cerr << "FAIL [silence]: output=" << out << " at sample " << i << '\n';
            return 1;
        }
    }
    return 0;
}

static int test_bypass_identity()
{
    ChannelDsp dsp;
    dsp.prepare(44100.0);
    dsp.setInstrument(0);
    dsp.setTargets(50.0f, true); // bypass on

    // Let the bypass mix settle to zero before checking identity.
    for (int i = 0; i < 20000; ++i)
        dsp.processSample(0.0f);

    std::mt19937 rng(123);
    std::uniform_real_distribution<float> dist(-0.9f, 0.9f);

    for (int i = 0; i < 1000; ++i)
    {
        float in = dist(rng);
        float out = dsp.processSample(in);
        if (std::abs(out - in) > 1e-5f)
        {
            std::cerr << "FAIL [bypass identity]: in=" << in << " out=" << out << " at sample " << i << '\n';
            return 1;
        }
    }
    return 0;
}

static int test_finitude()
{
    std::mt19937 rng(456);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (int inst = 0; inst < 2; ++inst)
    {
        for (float restore : { 0.0f, 50.0f, 100.0f })
        {
            ChannelDsp dsp;
            dsp.prepare(44100.0);
            dsp.setInstrument(inst);
            dsp.setTargets(restore, false);

            for (int i = 0; i < 50000; ++i)
            {
                float out = dsp.processSample(dist(rng));
                if (!finite(out))
                {
                    std::cerr << "FAIL [finitude]: restore=" << restore
                              << " inst=" << inst << " sample=" << i << " out=" << out << '\n';
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int test_restore_zero_is_conservative()
{
    ChannelDsp dsp;
    dsp.prepare(44100.0);
    dsp.setInstrument(0);
    dsp.setTargets(0.0f, false);

    constexpr float freq = 1000.0f;
    constexpr float amp = 0.5f;
    float inRms2 = 0.0f;
    float outRms2 = 0.0f;

    for (int i = 0; i < 44100; ++i)
    {
        float t = 2.0f * 3.14159265f * freq * static_cast<float>(i) / 44100.0f;
        float in = amp * std::sin(t);
        float out = dsp.processSample(in);

        if (i > 1000)
        {
            inRms2 += in * in;
            outRms2 += out * out;
        }
    }

    if (inRms2 <= 0.0f)
        return 1;

    float ratioDb = 10.0f * std::log10(outRms2 / inRms2);
    if (std::abs(ratioDb) > 1.0f)
    {
        std::cerr << "FAIL [restore=0]: RMS change is " << ratioDb << " dB\n";
        return 1;
    }
    return 0;
}

static int test_restore_change_no_clicks()
{
    ChannelDsp dsp;
    dsp.prepare(44100.0);
    dsp.setInstrument(0);
    dsp.setTargets(0.0f, false);

    constexpr float freq = 500.0f;
    constexpr float amp = 0.5f;
    float prev = 0.0f;
    float maxDelta = 0.0f;

    // pre-settle
    for (int i = 0; i < 5000; ++i)
    {
        float t = 2.0f * 3.14159265f * freq * static_cast<float>(i) / 44100.0f;
        prev = dsp.processSample(amp * std::sin(t));
    }

    dsp.setTargets(100.0f, false);

    for (int i = 0; i < 1000; ++i)
    {
        float t = 2.0f * 3.14159265f * freq * static_cast<float>(5000 + i) / 44100.0f;
        float out = dsp.processSample(amp * std::sin(t));
        float delta = std::abs(out - prev);
        if (delta > maxDelta)
            maxDelta = delta;
        prev = out;
    }

    // A discontinuity would stand out against the smooth 500 Hz sine.
    if (maxDelta > 0.3f)
    {
        std::cerr << "FAIL [restore change]: max sample delta=" << maxDelta << '\n';
        return 1;
    }
    return 0;
}

static int test_instrument_change_no_clicks()
{
    ChannelDsp dsp;
    dsp.prepare(44100.0);
    dsp.setInstrument(0);
    dsp.setTargets(0.0f, false);

    constexpr float freq = 500.0f;
    constexpr float amp = 0.5f;
    float prev = 0.0f;
    float maxDelta = 0.0f;

    for (int i = 0; i < 5000; ++i)
    {
        float t = 2.0f * 3.14159265f * freq * static_cast<float>(i) / 44100.0f;
        prev = dsp.processSample(amp * std::sin(t));
    }

    dsp.setInstrument(1);

    for (int i = 0; i < 1000; ++i)
    {
        float t = 2.0f * 3.14159265f * freq * static_cast<float>(5000 + i) / 44100.0f;
        float out = dsp.processSample(amp * std::sin(t));
        float delta = std::abs(out - prev);
        if (delta > maxDelta)
            maxDelta = delta;
        prev = out;
    }

    if (maxDelta > 0.3f)
    {
        std::cerr << "FAIL [instrument change]: max sample delta=" << maxDelta << '\n';
        return 1;
    }
    return 0;
}

static int test_restore_affects_output()
{
    constexpr int len = 44100;
    constexpr unsigned seed = 202;
    constexpr float thresholdDb = 0.5f;

    float rms0 = 0.0f;
    float rms100 = 0.0f;

    for (int run = 0; run < 2; ++run)
    {
        ChannelDsp dsp;
        dsp.prepare(44100.0);
        dsp.setInstrument(0);
        dsp.setTargets(run == 0 ? 0.0f : 100.0f, false);

        std::mt19937 rng(seed);
        std::uniform_real_distribution<float> dist(-0.5f, 0.5f);

        float rms2 = 0.0f;
        for (int i = 0; i < len; ++i)
        {
            float out = dsp.processSample(dist(rng));
            if (i > 1000)
                rms2 += out * out;
        }

        if (run == 0)
            rms0 = rms2;
        else
            rms100 = rms2;
    }

    if (rms0 <= 0.0f)
        return 1;

    float ratioDb = 10.0f * std::log10(rms100 / rms0);
    if (ratioDb < thresholdDb)
    {
        std::cerr << "FAIL [restore effect]: restore=100 changed level by only " << ratioDb << " dB\n";
        return 1;
    }
    return 0;
}

int main()
{
    struct Test
    {
        const char* name;
        int (*fn)();
    };

    Test tests[] = {
        { "silence", test_silence },
        { "bypass identity", test_bypass_identity },
        { "finitude", test_finitude },
        { "restore=0 conservative", test_restore_zero_is_conservative },
        { "restore change no clicks", test_restore_change_no_clicks },
        { "instrument change no clicks", test_instrument_change_no_clicks },
        { "restore affects output", test_restore_affects_output },
    };

    int failed = 0;
    for (const auto& t : tests)
    {
        std::cout << "Running " << t.name << "... " << std::flush;
        int r = t.fn();
        if (r == 0)
            std::cout << "OK\n";
        else
            ++failed;
    }

    if (failed > 0)
    {
        std::cerr << '\n' << failed << " test(s) failed\n";
        return 1;
    }

    std::cout << "\nAll tests passed\n";
    return 0;
}
