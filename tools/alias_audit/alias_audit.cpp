// alias_audit: measures non-harmonic output energy of a VST3 effect at hot settings. Usage: alias_audit plugin.vst3 [--json out.json]   or   alias_audit --selftest
// Method: coherent sampling (the sine fits a whole number of cycles in the analysis window, so a plain FFT has no leakage), 0.5 amplitude,
// 5 test frequencies chosen so that the sample rate is NOT a whole multiple of them (otherwise folded harmonics land exactly on other harmonics and aliasing is invisible). Everything in the output spectrum that is not a multiple of the input frequency counts as non-harmonic: aliasing
// from distortion, but also sidebands of modulation effects. Compare a plugin with its own earlier numbers, not with other plugins.
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <regex>
#include <cstdio>
#include <functional>

static bool hotName (const juce::String& n)
{
    static const std::regex hot ("drive|fuzz|dist|crush|fold|clip|sat|heat|warm|tube|tape|boost|push|amount|intensity|exciter|air|mix|wet|gain|input", std::regex::icase);
    static const std::regex skip ("bypass|mute|power|enable|\\bon\\b|\\boff\\b|output|master|ceiling|threshold", std::regex::icase);
    const std::string s = n.toStdString(); return std::regex_search (s, hot) && ! std::regex_search (s, skip);
}

struct Row { double hz; double nonHarmDb; double thdDb; };

static Row measure (const std::function<void (juce::AudioBuffer<float>&)>& process, const std::function<void()>& reset, double sr, double freq)
{
    constexpr int logN = 16, N = 1 << logN, block = 512; const int fbin = (int) std::lround (freq * N / sr); const double f = fbin * sr / N;
    const int warm = (int) sr;                                              // one second for smoothers and envelopes to settle
    juce::AudioBuffer<float> buf (2, block); std::vector<float> cap; cap.reserve ((size_t) N); long pos = 0;
    reset();
    while ((int) cap.size() < N)
    {
        for (int i = 0; i < block; ++i, ++pos) { const float v = 0.5f * (float) std::sin (2.0 * juce::MathConstants<double>::pi * f * (double) pos / sr); buf.setSample (0, i, v); buf.setSample (1, i, v); }
        process (buf);
        if (pos > warm) for (int i = 0; i < block && (int) cap.size() < N; ++i) cap.push_back (buf.getSample (0, i));
    }
    // the capture started on a block boundary, not on a cycle boundary: coherence needs the input phase continuous, which holds because pos keeps counting
    juce::dsp::FFT fft (logN); std::vector<float> data ((size_t) N * 2, 0.f); std::copy (cap.begin(), cap.end(), data.begin()); fft.performFrequencyOnlyForwardTransform (data.data());
    double total = 0, harm = 0, fund = 0;
    for (int b = 3; b < N / 2; ++b) { const double e = (double) data[(size_t) b] * data[(size_t) b]; total += e; const int k = (b + fbin / 2) / std::max (1, fbin); const bool onHarm = std::abs (b - k * fbin) <= 1 && k >= 1; if (onHarm) harm += e; if (std::abs (b - fbin) <= 1) fund += e; }
    const double non = std::max (1e-30, total - harm); Row r; r.hz = f;
    r.nonHarmDb = 10 * std::log10 (non / std::max (1e-30, total)); r.thdDb = 10 * std::log10 (std::max (1e-30, harm - fund) / std::max (1e-30, fund));
    return r;
}

// Positive control: the same hard drive through a naive tanh and through a 4x oversampled tanh. The metric must see the naive one as much worse.
static int selfTest()
{
    auto naive = [] (juce::AudioBuffer<float>& b) { for (int c = 0; c < b.getNumChannels(); ++c) { auto* x = b.getWritePointer (c); for (int i = 0; i < b.getNumSamples(); ++i) x[i] = std::tanh (12.0f * x[i]); } };
    juce::dsp::Oversampling<float> os (2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false); os.initProcessing (512);
    auto over = [&] (juce::AudioBuffer<float>& b)
    {
        juce::dsp::AudioBlock<float> blk (b); auto up = os.processSamplesUp (blk);
        for (size_t c = 0; c < up.getNumChannels(); ++c) { auto* x = up.getChannelPointer (c); for (size_t i = 0; i < up.getNumSamples(); ++i) x[i] = std::tanh (12.0f * x[i]); }
        os.processSamplesDown (blk);
    };
    int bad = 0;
    for (double hz : { 3100.0, 9100.0 })
    {
        const Row a = measure (naive, [] {}, 48000.0, hz), b = measure (over, [&] { os.reset(); }, 48000.0, hz);
        std::printf ("selftest %5.0f Hz: naive tanh %.1f dB, 4x oversampled %.1f dB\n", a.hz, a.nonHarmDb, b.nonHarmDb);
        if (! (a.nonHarmDb > b.nonHarmDb + 10.0)) { std::puts ("  FAIL: the oversampled version should be at least 10 dB cleaner"); ++bad; }
    }
    std::puts (bad ? "selftest FAILED" : "selftest passed"); return bad ? 1 : 0;
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init; if (argc > 1 && juce::String (argv[1]) == "--selftest") return selfTest();
    if (argc < 2) { std::puts ("usage: alias_audit plugin.vst3 [--json out.json]"); return 2; }
    juce::AudioPluginFormatManager fm; fm.addFormat (new juce::VST3PluginFormat());
    juce::OwnedArray<juce::PluginDescription> descs; fm.getFormat (0)->findAllTypesForFile (descs, argv[1]);
    if (descs.isEmpty()) { std::printf ("no plugin found in %s\n", argv[1]); return 0; }
    juce::String json = "[";
    for (auto* d : descs)
    {
        juce::String err; std::unique_ptr<juce::AudioPluginInstance> p (fm.createPluginInstance (*d, 48000.0, 512, err));
        if (! p) { std::printf ("%s: could not load (%s)\n", d->name.toRawUTF8(), err.toRawUTF8()); continue; }
        if (d->isInstrument) { std::printf ("%s: instrument, not measured (a synth is inharmonic by design)\n", d->name.toRawUTF8()); continue; }
        p->setPlayConfigDetails (2, 2, 48000.0, 512); p->prepareToPlay (48000.0, 512);
        std::vector<float> def; for (auto* prm : p->getParameters()) def.push_back (prm->getValue());
        int nHot = 0; for (auto* prm : p->getParameters()) if (hotName (prm->getName (64))) { prm->setValue (1.0f); ++nHot; }
        std::printf ("%s: %d drive-like parameters turned to maximum\n", d->name.toRawUTF8(), nHot);
        std::printf ("   test Hz   non-harmonic energy (dB, lower is cleaner)   harmonics vs fundamental (dB)\n");
        double worst = -300;
        for (double hz : { 1100.0, 3100.0, 7300.0, 9100.0, 11300.0 })
        {
            juce::MidiBuffer midi; const Row r = measure ([&] (juce::AudioBuffer<float>& b) { p->processBlock (b, midi); }, [&] { p->reset(); }, 48000.0, hz); worst = std::max (worst, r.nonHarmDb);
            std::printf ("   %7.0f   %8.1f                                      %8.1f\n", r.hz, r.nonHarmDb, r.thdDb);
            json << (json.length() > 1 ? "," : "") << "{\"plugin\":\"" << d->name << "\",\"hz\":" << r.hz << ",\"nonHarmDb\":" << r.nonHarmDb << ",\"harmDb\":" << r.thdDb << "}";
        }
        std::printf ("   worst non-harmonic energy: %.1f dB\n", worst);
        { int i = 0; for (auto* prm : p->getParameters()) prm->setValue (def[(size_t) i++]); }
    }
    json << "]";
    for (int i = 2; i + 1 < argc; ++i) if (juce::String (argv[i]) == "--json") juce::File (argv[i + 1]).replaceWithText (json);
    return 0;
}
