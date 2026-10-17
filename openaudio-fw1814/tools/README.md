# Bench tools (not shipped)

All run on the build PC (10.13.6) unless stated; build on the build PC (`scripts/sync-to-buildpc.sh` copies `tools/`, and **rsync --delete removes any
binary built inside `~/oa-build/tools`, so rebuild after each sync**):

    cc -o <name> <name>.c -framework CoreAudio -framework CoreFoundation -framework IOKit -framework CoreMIDI

| tool | what it does |
|---|---|
| `maprop` | read/write `MA*` properties through the same IOKit calls as the panel; `MA_CLASS=com_m_audio_FW1814Device` selects the official driver on the SL rig |
| `halwatch` | log CoreAudio HAL notifications of the FW1814 (what Audio MIDI Setup reacts to) |
| `aduplex` | duplex CoreAudio client that outputs silence and reports callbacks/peaks (streaming smoke test) |
| `halinfo` | latency numbers CoreAudio reports (device latency, safety offset), buffer range, available rates |
| `looplat` | **real** round-trip latency with an analog Out n -> In n loopback (`looplat [RATE [BUFFER [SECONDS]]]`) |
| `ratecycle` | walks the interface through a list of rates with CoreAudio and prints status + in/out channel counts after each; **the same tool runs on the SL rig** |
| `midiprobe` | CoreMIDI list/send/listen/loop, shows the device image path, `removedev` drops a stale device from the MIDI setup |
| `mafw-start.c` | firmware start helper from the early bring-up |

## Running on the Snow Leopard rig (`ssh fw1814-sl`, official driver v1.9, 10.6.8, no compiler)
Build on the build PC (Xcode 9.4.1 still targets 10.6 for x86_64; i386 is gone):

    cc -w -arch x86_64 -mmacosx-version-min=10.6 -o ratecycle-sl ratecycle.c -framework CoreAudio -framework CoreFoundation

then `scp` it to the SL into the persistent folder **`~/fw1814-captures/tools/`** (never `/tmp`). Run only while nothing streams (a rate change while a client streams
panics v1.9). The SL HAL applies a rate request only while the client's run loop spins (`ratecycle` already does that). Put the SL back to the rate it had (48 kHz).
Example: `~/fw1814-captures/tools/ratecycle 48000 192000 48000` -> capture 85.

## Reading the clone's kernel diagnostics
The build PC's `dmesg` ring and unified log lose this kext's `IOLog` lines under the IOAudio debug flood. The kext therefore keeps a ring of format-change events in the
registry property **`MADebugTrace`**: `ioreg -r -c com_openaudio_FW1814Device -l | grep -A20 MADebugTrace`.
