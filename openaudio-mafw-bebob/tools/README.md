# Bench tools (not shipped)

All run on the build PC (10.13.6) unless stated; build on the build PC (`scripts/sync-to-buildpc.sh` copies `tools/`, and **rsync --delete removes any
binary built inside `~/oa-build/tools`, so rebuild after each sync**):

    cc -o <name> <name>.c -framework CoreAudio -framework CoreFoundation -framework IOKit -framework CoreMIDI

| tool | what it does |
|---|---|
| `maprop` | read/write `MA*` properties through the same IOKit calls as the panel; `MA_CLASS=com_m_audio_FW1814Device` selects the official driver on the SL rig; `maprop allxml` prints `MAMetaAllProperties` as an XML plist; `maprop setxml FILE` writes such a dump back with one `setProperties` (as the helper's replay; rate/lock/bandwidth/engine-option and `MAMeta*` keys left out); with two units, `MA_CLASS=com_openaudio_FWAudiophileDevice` (or `…FW1814Device`) picks one |
| `factory-diff.py` | compares a `maprop allxml` dump taken after `MAMetaResetToFactorySettings` with the official driver's settings file after its factory reset (capture 92 for the Audiophile); acceptance of AP-3.6 in the hardware run |
| `halwatch` | log CoreAudio HAL notifications of the FW1814 (what Audio MIDI Setup reacts to) |
| `aduplex` | duplex CoreAudio client that outputs silence and reports callbacks/peaks (streaming smoke test) |
| `halinfo` | latency numbers CoreAudio reports (device latency, safety offset), buffer range, available rates |
| `looplat` | **real** round-trip latency with an analog Out n -> In n loopback (`looplat [RATE [BUFFER [SECONDS]]]`) |
| `ratecycle` | walks the interface through a list of rates with CoreAudio and prints status + in/out channel counts after each; **the same tool runs on the SL rig** |
| `midiprobe` | CoreMIDI list/send/listen/loop, shows the device image path, `removedev` drops a stale device from the MIDI setup |
| `mafw-start.c` | firmware start helper from the early bring-up |

## Two units attached
`halinfo`, `ratecycle`, `loopcheck`, `looplat` and `aduplex` take the first CoreAudio device whose UID contains `FWEngine2`; with both units attached set
**`OA_DEV=<part of the UID>`** — the UID is `com_m_audio_FWEngine2:0x<GUID>`, so the GUID picks a unit (`OA_DEV=d6c0300211d54` = the Audiophile,
`OA_DEV=d6c040012337f` = the bench FW1814; case-insensitive). The phase-7 session script `tests/hw-smoke/phase7.sh` (with `tests/hw-smoke/p7util.py`)
builds these tools into `tools/build/` on the build PC and sets `OA_DEV` / `MA_CLASS` from `UNIT=`.

## Panel CPU bench (`panel-cpubench.sh`, R119)
`panel-cpubench.sh "<label>" [panel args]` runs a demo panel for ~25 s and prints the panel's and WindowServer's average CPU (`top`, 5 s samples). Panel bench options (demo mode only): `--big-meter` opens the big meter, `--no-panel` hides the panel windows. Examples: `panel-cpubench.sh "mixer"`, `panel-cpubench.sh "big meter" --big-meter --no-panel`. Keep the screen awake and unlocked (`caffeinate -d -t 1800`): hidden or covered windows stop drawing by design.

## Running on the MacBook Air (`192.168.100.22`, 10.13.6)
The Air has no Xcode and `sudo` there needs a password. Build the tools on the build PC (`cc -w -mmacosx-version-min=10.13 -o <name> <name>.c -framework CoreAudio -framework CoreFoundation -framework IOKit -framework CoreMIDI`), copy the binaries to `~/fw1814-captures/tools/` on the Air (persistent, not `/tmp`) and run them over SSH. They need no root; loading kexts and installing packages does (the user does that locally).

## Running on the Snow Leopard rig (`ssh fw1814-sl`, official driver v1.9, 10.6.8, no compiler)
Build on the build PC (Xcode 9.4.1 still targets 10.6 for x86_64; i386 is gone):

    cc -w -arch x86_64 -mmacosx-version-min=10.6 -o ratecycle-sl ratecycle.c -framework CoreAudio -framework CoreFoundation

then `scp` it to the SL into the persistent folder **`~/fw1814-captures/tools/`** (never `/tmp`). Run only while nothing streams (a rate change while a client streams
panics v1.9). The SL HAL applies a rate request only while the client's run loop spins (`ratecycle` already does that). Put the SL back to the rate it had (48 kHz).
Example: `~/fw1814-captures/tools/ratecycle 48000 192000 48000` -> capture 85.

`slfwread` (read-only status-packet reads, nothing written): `status [N [ms]]` = the Audiophile's 60-byte packet (capture 93); `block1814 [N [ms]]`
= the FW1814's 88-byte packet, word 20 decoded as the kext does (rate field byte 0x52, lock byte 0x53; capture 98, R131); `quad HI LO`. Built on the
dev Mac (15.2 SDK) it needs `clang -arch x86_64 -mmacosx-version-min=10.6 -fno-stack-protector -D_FORTIFY_SOURCE=0 -Wl,-no_pie slfwread.c -framework
IOKit -framework CoreFoundation` (without those flags it segfaults at start on 10.6.8); on the rig as `~/fw1814-captures/tools/slfwread-b1814`.

## Reading the clone's kernel diagnostics
The build PC's `dmesg` ring and unified log lose this kext's `IOLog` lines under the IOAudio debug flood. The kext therefore keeps a ring of format-change events in the
registry property **`MADebugTrace`**: `ioreg -r -c com_openaudio_FWAVCAudioDevice -l | grep -A20 MADebugTrace`.
