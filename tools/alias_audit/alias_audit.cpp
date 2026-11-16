// alias_audit: estimates the aliasing of a VST3 effect at hot settings. Usage: alias_audit plugin.vst3 [--json out.json]   or   alias_audit --selftest
// Method: a sine (coherent sampling, so a plain FFT has no leakage) goes through the plugin at 48 kHz and again at 192 kHz.
// Energy that is not a multiple of the input frequency, below 20 kHz, is measured at both rates. Modulation sidebands and noise show up at both rates;
// aliasing folds back into the band mostly at 48 kHz. So  aliasing = (non-harmonic energy at 48 kHz) minus (non-harmonic energy at 192 kHz).
// Both numbers are reported in dB relative to the total output energy in the band. It is an estimate: a plugin whose behaviour depends on the sample rate
// in other ways (filters, smoothing) can read slightly off. Compare a plugin with its own earlier numbers.
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

struct Spectrum { double total = 0, nonHarm = 0; };
using Process = std::function<void (juce::AudioBuffer<float>&)>;

// Runs the sine through `process` and measures energy below 20 kHz. N is chosen per rate so that bin spacing (sr / N) is the same at every rate.
static Spectrum measure (const Process& process, const std::function<void()>& reset, double sr, double freq)
{
    const int logN = (sr > 100000 ? 18 : 16), N = 1 << logN, block = 512; const double binHz = sr / N; const int fbin = (int) std::lround (freq / binHz);
    const double f = fbin * binHz, warm = sr;                                   // one second for smoothers and envelopes to settle
    juce::AudioBuffer<float> buf (2, block); std::vector<float> cap; cap.reserve ((size_t) N); long pos = 0;
    reset();
    while ((int) cap.size() < N)
    {
        for (int i = 0; i < block; ++i, ++pos) { const float v = 0.5f * (float) std::sin (2.0 * juce::MathConstants<double>::pi * f * (double) pos / sr); buf.setSample (0, i, v); buf.setSample (1, i, v); }
        process (buf);
        if ((double) pos > warm) for (int i = 0; i < block && (int) cap.size() < N; ++i) cap.push_back (buf.getSample (0, i));
    }
    juce::dsp::FFT fft (logN); std::vector<float> data ((size_t) N * 2, 0.f); std::copy (cap.begin(), cap.end(), data.begin()); fft.performFrequencyOnlyForwardTransform (data.data());
    Spectrum s; const int top = std::min (N / 2, (int) (20000.0 / binHz));
    for (int b = 3; b < top; ++b)
    {
        const double m = (double) data[(size_t) b] / N, e = m * m; s.total += e;                   // divided by N: power on the same scale at every FFT size
        const int k = (b + fbin / 2) / std::max (1, fbin); const bool onHarm = k >= 1 && std::abs (b - k * fbin) <= 1; if (! onHarm) s.nonHarm += e;
    }
    return s;
}

struct Row { double hz, aliasDb, otherDb; };
static Row measureBoth (const std::function<Process (double)>& make, const std::function<void (double)>& prepare, double freq)
{
    prepare (48000.0); const Spectrum a = measure (make (48000.0), [] {}, 48000.0, freq);
    prepare (192000.0); const Spectrum b = measure (make (192000.0), [] {}, 192000.0, freq);
    // comparable because the energies are normalised by the FFT size and the bin spacing and the 20 kHz limit are the same at both rates
    const double ref = std::max (1e-30, a.total), alias = std::max (a.nonHarm - b.nonHarm, 1e-12 * ref);
    return { freq, 10 * std::log10 (alias / ref), 10 * std::log10 (std::max (b.nonHarm, 1e-12 * ref) / ref) };
}

// Positive control: the same hard drive through a naive tanh and through a 4x oversampled tanh.
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
        const Row a = measureBoth ([&] (double) { return Process (naive); }, [] (double) {}, hz);
        const Row b = measureBoth ([&] (double) { return Process (over); }, [&] (double) { os.reset(); }, hz);
        std::printf ("selftest %5.0f Hz: naive tanh aliasing %.1f dB, 4x oversampled %.1f dB\n", hz, a.aliasDb, b.aliasDb);
        if (! (a.aliasDb > b.aliasDb + 10.0)) { std::puts ("  FAIL: the oversampled version should be at least 10 dB cleaner"); ++bad; }
    }
    std::puts (bad ? "selftest FAILED" : "selftest passed"); return bad ? 1 : 0;
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init; if (argc > 1 && juce::String (argv[1]) == "--selftest") return selfTest();
    if (argc < 2) { std::puts ("usage: alias_audit plugin.vst3 [--json out.json]   or   alias_audit --selftest"); return 2; }
    juce::AudioPluginFormatManager fm; fm.addFormat (new juce::VST3PluginFormat());
    juce::OwnedArray<juce::PluginDescription> descs; fm.getFormat (0)->findAllTypesForFile (descs, argv[1]);
    if (descs.isEmpty()) { std::printf ("no plugin found in %s\n", argv[1]); return 0; }
    juce::String json = "[";
    for (auto* d : descs)
    {
        juce::String err; std::unique_ptr<juce::AudioPluginInstance> p (fm.createPluginInstance (*d, 48000.0, 512, err));
        if (! p) { std::printf ("%s: could not load (%s)\n", d->name.toRawUTF8(), err.toRawUTF8()); continue; }
        if (d->isInstrument) { std::printf ("%s: instrument, not measured (a synth is inharmonic by design)\n", d->name.toRawUTF8()); continue; }
        std::vector<float> def; for (auto* prm : p->getParameters()) def.push_back (prm->getValue());
        int nHot = 0; for (auto* prm : p->getParameters()) if (hotName (prm->getName (64))) { prm->setValue (1.0f); ++nHot; }
        std::printf ("%s: %d drive-like parameters turned to maximum\n", d->name.toRawUTF8(), nHot);
        std::printf ("   test Hz   aliasing (dB, lower is cleaner)   other non-harmonic: modulation, noise (dB)\n");
        double worst = -300; juce::MidiBuffer midi;
        for (double hz : { 1100.0, 3100.0, 7300.0, 9100.0, 11300.0 })
        {
            const Row r = measureBoth ([&] (double) { return Process ([&] (juce::AudioBuffer<float>& b) { p->processBlock (b, midi); }); },
                                       [&] (double sr) { p->releaseResources(); p->setPlayConfigDetails (2, 2, sr, 512); p->prepareToPlay (sr, 512); }, hz);
            worst = std::max (worst, r.aliasDb);
            std::printf ("   %7.0f   %8.1f                         %8.1f\n", r.hz, r.aliasDb, r.otherDb);
            json << (json.length() > 1 ? "," : "") << "{\"plugin\":\"" << d->name << "\",\"hz\":" << r.hz << ",\"aliasDb\":" << r.aliasDb << ",\"otherDb\":" << r.otherDb << "}";
        }
        std::printf ("   worst aliasing: %.1f dB\n", worst);
        { int i = 0; for (auto* prm : p->getParameters()) prm->setValue (def[(size_t) i++]); }
    }
    json << "]";
    for (int i = 2; i + 1 < argc; ++i) if (juce::String (argv[i]) == "--json") juce::File (argv[i + 1]).replaceWithText (json);
    return 0;
}
