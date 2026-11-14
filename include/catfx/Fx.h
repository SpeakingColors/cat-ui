// FX rack for synths and instruments: shimmer delay into reverb (plate, hall, shimmer plate, shimmer hall).
// Header-only, no JUCE dependency, no allocation after prepare(). See Reverb.h for the algorithms and credits.
// Knob changes are smoothed inside the rack, so a host automation step or a fast knob move glides instead of clicking.
#pragma once
#include <cmath>
#include "Reverb.h"

namespace catfx {

struct FxRackParams {
    bool delayOn = false, reverbOn = false;
    ShimmerDelayParams delay;
    ReverbSettings reverb;
};

class FxRack {
public:
    void prepare(float sampleRate) {
        sr_ = sampleRate;
        delay_.prepare(sampleRate); reverb_.prepare(sampleRate);
        fast_ = 1.0f - std::exp(-1.0f / (0.020f * sampleRate));   // about 20 ms: levels and tone
        slow_ = 1.0f - std::exp(-1.0f / (0.060f * sampleRate));   // about 60 ms: delay time and pitch, so they glide without zipper noise
        dPrimed_ = rPrimed_ = false;
    }
    void reset() { delay_.reset(); reverb_.reset(); dPrimed_ = rPrimed_ = false; }

    // In place on a stereo buffer.
    void process(float* l, float* r, int n, const FxRackParams& p) {
        // A section that was off starts straight at its target. One that is switching off fades its mix out first.
        if (p.delayOn) { if (!dPrimed_) { dS_ = p.delay; dPrimed_ = true; } }
        if (p.reverbOn) { if (!rPrimed_) { rS_ = p.reverb; rPrimed_ = true; } }
        ShimmerDelayParams dT = p.delay; ReverbSettings rT = p.reverb;
        if (!p.delayOn) dT.mix = 0.0f;
        if (!p.reverbOn) rT.mix = 0.0f;
        const bool dRun = dPrimed_, rRun = rPrimed_;
        if (!dRun && !rRun) return;
        for (int i = 0; i < n; ++i) {
            float a = l[i], b = r[i];
            if (dRun) { glide(dS_, dT); delay_.process(a, b, a, b, dS_); }
            if (rRun) { glide(rS_, rT); reverb_.process(a, b, a, b, rS_); }
            l[i] = a; r[i] = b;
        }
        if (dPrimed_ && !p.delayOn && dS_.mix < 0.001f) dPrimed_ = false;
        if (rPrimed_ && !p.reverbOn && rS_.mix < 0.001f) rPrimed_ = false;
    }
private:
    // One step of smoothing toward the target; snaps when close so no denormals build up.
    void step(float& s, float t, float a) const {
        const float d = t - s;
        s = (std::fabs(d) < 1e-6f * (1.0f + std::fabs(t))) ? t : s + a * d;
    }
    void glide(ShimmerDelayParams& s, const ShimmerDelayParams& t) const {
        step(s.timeMs, t.timeMs, slow_); step(s.shiftSemis, t.shiftSemis, slow_);
        step(s.feedback, t.feedback, fast_); step(s.dampHz, t.dampHz, fast_);
        step(s.mix, t.mix, fast_); step(s.shimmerAmount, t.shimmerAmount, fast_);
    }
    void glide(ReverbSettings& s, const ReverbSettings& t) const {
        s.type = t.type;                                            // the type is a switch, it cannot be blended
        step(s.p.decay, t.p.decay, fast_); step(s.p.dampHz, t.p.dampHz, fast_);
        step(s.p.preDelayMs, t.p.preDelayMs, slow_); step(s.p.size, t.p.size, slow_);
        step(s.p.modDepth, t.p.modDepth, fast_);
        step(s.shimmerSemis, t.shimmerSemis, slow_); step(s.shimmerAmount, t.shimmerAmount, fast_);
        step(s.mix, t.mix, fast_);
    }

    float sr_ = 48000, fast_ = 0.001f, slow_ = 0.0003f;
    bool dPrimed_ = false, rPrimed_ = false;
    ShimmerDelayParams dS_;
    ReverbSettings rS_;
    ShimmerDelay delay_;
    ReverbUnit reverb_;
};

}  // namespace catfx
