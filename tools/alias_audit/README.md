# alias_audit

Loads a built VST3 effect, turns its drive-like knobs (names with drive, fuzz, dist, crush, fold, clip, sat, warm, tube, tape, boost, mix, wet, gain...) to maximum, feeds it sine waves and reports how much of the output energy is not a harmonic of the input.

- Aliasing from distortion lands on non-harmonic frequencies, so a high number means more aliasing. Modulation effects (chorus, phaser) also create non-harmonic sidebands, so compare a plugin with its own earlier numbers, not with other plugins.
- Test frequencies (1.1, 3.1, 7.3, 9.1, 11.3 kHz) are chosen so the sample rate is not a whole multiple of them. At 3 kHz and 48 kHz folded harmonics land exactly on other harmonics and aliasing would be invisible. Found with the self test.
- Instruments are skipped (a synth is inharmonic by design). It never fails a build; it prints a table.

```
cmake -S tools/alias_audit -B build-alias -DCMAKE_BUILD_TYPE=Release && cmake --build build-alias --parallel 4
xvfb-run -a build-alias/alias_audit_artefacts/Release/alias_audit path/to/Plugin.vst3
build-alias/alias_audit_artefacts/Release/alias_audit --selftest     # naive tanh vs 4x oversampled tanh, must differ by 10 dB or more
```

Reference points from the self test at 48 kHz, tanh(12x): naive -24.6 dB at 3.1 kHz and -9.5 dB at 9.1 kHz; 4x oversampled -103 dB and -47 dB.
