// FX rack for synths and instruments: shimmer delay into reverb (plate, hall, shimmer plate, shimmer hall).
// Header-only, no JUCE dependency, no allocation after prepare(). See Reverb.h for the algorithms and credits.
#pragma once
#include "Reverb.h"

namespace catfx {

struct FxRackParams {
    bool delayOn = false, reverbOn = false;
    ShimmerDelayParams delay;
    ReverbSettings reverb;
};

class FxRack {
public:
    void prepare(float sampleRate) { delay_.prepare(sampleRate); reverb_.prepare(sampleRate); }
    void reset() { delay_.reset(); reverb_.reset(); }
    // In place on a stereo buffer.
    void process(float* l, float* r, int n, const FxRackParams& p) {
        if (!p.delayOn && !p.reverbOn) return;
        for (int i = 0; i < n; ++i) {
            float a = l[i], b = r[i];
            if (p.delayOn) delay_.process(a, b, a, b, p.delay);
            if (p.reverbOn) reverb_.process(a, b, a, b, p.reverb);
            l[i] = a; r[i] = b;
        }
    }
private:
    ShimmerDelay delay_;
    ReverbUnit reverb_;
};

}  // namespace catfx
