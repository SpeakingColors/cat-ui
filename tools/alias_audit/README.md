# alias_audit

Loads a built VST3 effect, turns its drive-like knobs (names with drive, fuzz, dist, crush, fold, clip, sat, warm, tube, tape, boost, mix, wet, gain...) to maximum and estimates how much it aliases.

## Method
A sine (coherent sampling, so a plain FFT has no leakage) goes through the plugin at 48 kHz and again at 192 kHz. Energy that is not a multiple of the input frequency, below 20 kHz, is measured at both rates. Modulation sidebands and noise appear at both rates; aliasing folds back into the band mostly at 48 kHz. So:

- **aliasing** = non-harmonic energy at 48 kHz minus non-harmonic energy at 192 kHz (dB relative to the output energy in the band, lower is cleaner)
- **other non-harmonic** = what is left at 192 kHz: modulation, noise, anything that is not a harmonic

It is an estimate. A plugin whose behaviour depends on the sample rate in other ways (filters, smoothing) can read slightly off. Compare a plugin with its own earlier numbers, and remember that all drive-like knobs at maximum is an extreme setting. Instruments are skipped (a synth is inharmonic by design). The tool never fails a build; it prints a table.

## Test frequencies
1.1, 3.1, 7.3, 9.1 and 11.3 kHz are chosen so the sample rate is not a whole multiple of them. At 3 kHz and 48 kHz the folded harmonics land exactly on other harmonics and aliasing would be invisible. This was found with the self test, and an earlier single rate version of the metric was wrong for the same reason it could not tell aliasing from modulation.

## Use
```
cmake -S tools/alias_audit -B build-alias -DCMAKE_BUILD_TYPE=Release && cmake --build build-alias --parallel 4
xvfb-run -a build-alias/alias_audit_artefacts/Release/alias_audit path/to/Plugin.vst3
build-alias/alias_audit_artefacts/Release/alias_audit --selftest     # naive tanh vs 4x oversampled tanh, must differ by 10 dB or more
```

## Reference points (measured, tanh(12x) test signal at 0.5 amplitude)
| | 3.1 kHz | 9.1 kHz |
|---|---|---|
| naive tanh | -29.1 dB | -14.7 dB |
| 4x oversampled (polyphase IIR) | -106 dB | -47 dB |

Scaredy Cat (2x oversampled drive stages) at all drive-like knobs maximum: -29.8 dB at 1.1 kHz, -22.6 at 3.1, -17.3 at 7.3, -14.8 at 9.1, -13.7 at 11.3 kHz; other non-harmonic about -6 dB at every frequency.
