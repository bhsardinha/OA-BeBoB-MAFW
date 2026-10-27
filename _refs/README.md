# _refs — third-party reference sources (git-ignored, rebuildable)

Clones and scratch unpacks cited by `docs/system-map/` (B1, B2, B2a, B3, B4). Everything here except this README is git-ignored. Rebuild it with the recipes below.

Until 2026-10-14 these files lived in `/tmp` and were lost when the old Mac rebooted. Recipes and timestamps were recovered from the session transcripts. Each clone is pinned to the last upstream commit before the original clone time, and spot-checked against the line citations in the docs (B1 `special_avdevice.cpp:41`, `cip.h:110`; B2 `bebob.c:174`, `bebob_maudio.c:196`; B2a `special.rs` = 1205 lines, `:308`).

| Path | Was | Upstream | Pinned commit | Original clone (UTC) |
|---|---|---|---|---|
| `ffado/` | `/tmp/ffado-ref/ffado/` | `https://github.com/ojsl1/libffado.git` (mirror; layout `src/bebob/maudio/…`) | `b74cd25b3a1879bb3c2fae51f7ad298fcd7380a8` (2025-10-01) | 2026-10-13 20:56 — no commits since, so identical to the original |
| `snd-firewire-ctl-services/` | `/tmp/snd-firewire-ctl-services/` | `https://github.com/alsa-project/snd-firewire-ctl-services.git` | `7654d40cb0e62426e460c0b94605e2a4b506d6bb` (2026-09-10) | 2026-10-09 19:53 — no commits since, so identical |
| `linux-snd-bebob/` | `/tmp/linux-snd-bebob/` | `https://github.com/torvalds/linux.git`, sparse `sound/firewire/bebob/*`, `sound/firewire/*.{c,h}` | `e90ab3669181c6469e3647ef2b30ae408275f496` (2026-10-12) | 2026-10-13 20:55 (`master`, depth 1) |
| `sierra_kext_unpack/` | `/tmp/sierra_kext_unpack/` | `_vendor/M-AUDIO FWAUDIOPHILE KEXT-LOAD SIERRA/1_…/M-Audio FireWire.mpkg/Contents/Resources/packages/M-Audio FireWire Kernel Extension.pkg` | — (same output every run) | B3, 2026-10-13 |
| `macfw/` | `_extracted/macfw/` (moved 2026-10-14; B4 + C6 citations updated) | `https://github.com/mbprado/macfw.git` | `054e08bb6dd0f8238b942c8d305cc73edd535052` (2026-09-29) | copied from the old Mac's repo 2026-10-14 (same commit) |
| `IOFireWireAVC-436/` | — | `https://github.com/apple-oss-distributions/IOFireWireAVC` (Apple open source; plist 4.2.8 = the version loaded on 14.8.9) | tag `IOFireWireAVC-436` = `c11f37b85360` | 2026-10-22, R133 (only `IOFireWireAVC/IOFireWireAVCUnit.cpp`, `IOFireWireAVCCommand.cpp`, `Info-IOFireWireAVC.plist`) |

## Recipes

```sh
cd _refs
git clone --filter=blob:none https://github.com/ojsl1/libffado.git ffado && git -C ffado checkout b74cd25b3a18
git clone --filter=blob:none https://github.com/alsa-project/snd-firewire-ctl-services.git && git -C snd-firewire-ctl-services checkout 7654d40cb0e6
git clone https://github.com/mbprado/macfw.git && git -C macfw checkout 054e08bb6dd0
mkdir -p IOFireWireAVC-436/IOFireWireAVC && for f in IOFireWireAVC/IOFireWireAVCUnit.cpp IOFireWireAVC/IOFireWireAVCCommand.cpp Info-IOFireWireAVC.plist; do curl -sfL "https://raw.githubusercontent.com/apple-oss-distributions/IOFireWireAVC/IOFireWireAVC-436/$f" -o "IOFireWireAVC-436/$f"; done
git clone --filter=blob:none --no-checkout --sparse --shallow-since=2026-10-01 https://github.com/torvalds/linux.git linux-snd-bebob
( cd linux-snd-bebob && git sparse-checkout set --no-cone 'sound/firewire/bebob/*' 'sound/firewire/*.c' 'sound/firewire/*.h' && git checkout e90ab3669181 )
mkdir sierra_kext_unpack && cd sierra_kext_unpack
xar -xf "../../_vendor/M-AUDIO FWAUDIOPHILE KEXT-LOAD SIERRA/1_M-Audio_FireWire_El_Capitan_Sierra_Install_Uninstall/M-Audio FireWire.mpkg/Contents/Resources/packages/M-Audio FireWire Kernel Extension.pkg"
mkdir payload_out scripts_out
(cd payload_out && gunzip -c ../Payload | cpio -i) && (cd scripts_out && gunzip -c ../Scripts | cpio -i)
```

**Note:** the v1.10.5 kext from the installer contains three architectures (x86_64 + i386 + ppc), with MD5 `8434e526…`. Only its x86_64 slice (`lipo -thin x86_64`) matches the Sierra hack binaries (`61a60a46…`). See the B3 correction.

## Other evidence that moved out of `/tmp`

| Was | Now | How |
|---|---|---|
| `/tmp/mafw.asm` | `docs/system-map/A1-ghidra/otool-exports/mafw.asm` | `otool -tV _extracted/kext/M-AudioFireWireBeBoB.x86_64-slice`. Same output every run; cited line numbers verified. |
| (Q11 `__DATA __const` dump) | `docs/system-map/A1-ghidra/otool-exports/mafw-data-const.txt` | `otool -v -s __DATA __const` on the same slice |
| `/tmp/midi.x64`, `/tmp/midi.i386` | `_extracted/midi/MAudioFireWireMIDI.{x86_64,i386}-slice` | `lipo -thin` of the plugin binary |
| `/tmp/clock-bijection.log`, `dispatcher-mute.log`, `q14.c`, `q15.c` | `docs/system-map/A1-ghidra/ghidra-exports/decomp/` | Rebuilt byte-for-byte from the full Read results in the session transcripts |
| `/tmp/a2-manual-crosscheck.log` | `docs/system-map/A2-panel/ghidra-exports/decomp/` | Lines 49–148 and 216–841 recovered from the transcripts. Lines 149–215 were filled from a Ghidra 12.1.4 re-run, which matched all 726 recovered lines exactly at offset −4. Lines 1–48 (Ghidra startup messages) are placeholders. |
| `/tmp/q13.out` | `…/A1-ghidra/ghidra-exports/decomp/q13.regenerated-2026-10-14.out` | Ghidra re-run only, so not the original bytes. No doc cites it by line. |

**Warning:** fresh Ghidra 12.1.4 re-runs of `DecompClockBijection.java` do **not** reproduce the original line numbers. Use the recovered files for line citations.


## macos-10.13.6-fw/ (added 2026-10-14)

Read-only copies from the build PC for Q-OHCI-interrupt. Recipe: `mkdir _refs/macos-10.13.6-fw && scp fw1814-build:/System/Library/Extensions/IOFireWireFamily.kext/Contents/PlugIns/AppleFWOHCI.kext/Contents/MacOS/AppleFWOHCI AppleFWOHCI.bin && scp fw1814-build:/System/Library/Extensions/IOFireWireFamily.kext/Contents/MacOS/IOFireWireFamily IOFireWireFamily.bin`; `otool -tvV AppleFWOHCI.bin > AppleFWOHCI.asm`. sha256 AppleFWOHCI `3d66e5db…ca243`, IOFireWireFamily `23594cd5…d168`. Also cloned: `IOFireWireFamily` (`git clone --depth 50 https://github.com/apple-oss-distributions/IOFireWireFamily.git`, commit `ad9f879`); `AppleFWOHCI` is not published (404).
