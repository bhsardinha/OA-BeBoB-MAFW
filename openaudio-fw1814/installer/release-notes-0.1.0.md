## OpenAudio FW1814 0.1.0 — first public version (early, 1814-only)

A native 64-bit driver, helper daemon, control panel and installer that make the **M-Audio FireWire 1814** work on **macOS 10.13.6 High Sierra**. M-Audio dropped support for these interfaces more than a decade ago and their last driver does not work on current macOS.

**Only for owners of an M-Audio FireWire 1814.** It does nothing without the interface. Homebrew and experimental: the kext is unsigned (and will stay so), so **System Integrity Protection must be disabled**. Tested on one FW1814, on two Macs running macOS 10.13.6 (an iMac, and a MacBook Air through Apple's Thunderbolt-to-FireWire adapter), SIP disabled — nothing else.

### What works

- Audio through Core Audio, 44.1–192 kHz: 16 in / 12 out (ADAT mode), 10 in / 6 out (S/PDIF mode), 2 in / 4 out analog at 176.4 and 192 kHz. Round-trip latency 9.3 ms at 48 kHz / 64 frames, measured.
- Control panel: mixer, outputs, phones, routing, clock, digital ports, meters, presets, saved settings.
- MIDI through M-Audio's own CoreMIDI plug-in (included unmodified).

### Install

Open the disk image, run **Install OpenAudio FW1814.pkg**, restart. To remove everything, run **Uninstall OpenAudio FW1814** from the same image. Quit audio apps first and remove any official M-Audio or Avid FireWire driver.

### Known issues (read before installing)

- One kernel panic was seen during playback (cause not proven; a likely cause was fixed, not yet confirmed). Do not use it for work you cannot afford to lose.
- Do not change the sample rate or the digital mode while audio is playing.
- MIDI on the wire, sleep/wake, bus-reset recovery, AC3 and multiple interfaces are untested.
- The Audio MIDI Setup sidebar label ("N ins / M outs") can be stale after an S/PDIF/ADAT switch.

Full list, screenshots and roadmap are in the README. License: PolyForm Noncommercial 1.0.0. The original M-Audio software and firmware belong to M-Audio / inMusic; see NOTICE.md.
