# Roadmap: cat-ui

Shared look and feel.

## Next
- [ ] Shared Settings and MIDI and help pages (see docs/mockups) as reusable components
- [ ] Gear and info buttons, backup/preset folder pickers
- [ ] Knob, fader, X-Y pad and envelope widgets used by every plugin

## Every cat plugin
- Draw through `catui::RenderGate` so meters and animations stop when the window is not in front (audio, automation and MIDI keep running)
- Settings page with MIDI filters on by default, selectable preset/backup folders, UI size reset to 100%
