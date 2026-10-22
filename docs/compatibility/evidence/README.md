# BROAD_COMPATIBILITY/evidence

Raw outputs behind `BROAD_COMPATIBILITY_PLAN_v1.md` (§2) and the `Evidence` tab of `BROAD_COMPATIBILITY_PLAN_v1.xlsx`. All were produced on 2026-10-21 by static reads: nothing was loaded, installed or changed on any machine.

| File | What it shows | Evidence id |
|---|---|---|
| `kext-0.2.2-imports.txt` | The 777 undefined symbols of the shipped 0.2.2 kext (`dist/OA-BeBoB-MAFW-0.2.2.pkg`, x86_64, built with Xcode 9.4.1 and the 10.13 SDK) | E-1 |
| `kext-0.2.2-imports-not-in-reference-builds.txt` | The 28 of those that no reference build imports (v1.9 from the SL rig, v1.10.5, v1.11.3) | E-1 |
| `kext-0.2.2-vs-macos-14.8.4-x86_64.txt` | The 2 imports not defined in the macOS 14.8.4 Intel kernel and families | E-4 |
| `buildpc-10.13.6-kpi-exports.txt` | 10.13.6 KPI symbol sets: lock, fortify, thread and version symbols; family versions; AppleFWAudio present | E-3, E-8 |
| `kdk-14.8.4-families-arm64e.txt` | macOS 14.8.4 KDK: the FireWire and IOAudio families are built for arm64e and sit in the Apple Silicon kernel caches | E-5 |
| `panel-0.2.2-demo-macos-14.8.9.txt` | The 0.2.2 panel starts and renders in demo mode on Intel macOS 14.8.9 | E-10 |

## Recipes

Paths are relative to the repository root. `$S` is any scratch folder.

```sh
# E-1  imports of the shipped kext, and the ones no reference build uses
pkgutil --expand-full openaudio-mafw-bebob/dist/OA-BeBoB-MAFW-0.2.2.pkg $S/x
nm -u "$S/x/component.pkg/Payload/Library/Extensions/OpenAudioBeBoBMAFW.kext/Contents/MacOS/OpenAudioBeBoBMAFW" | sort -u > $S/ours.u
nm -u _refs/sl-v1.9/v19.x86_64 | sort -u > $S/v19.u
lipo -thin x86_64 "$(find _refs/sierra_kext_unpack/payload_out -path '*MacOS/M-AudioFireWireBeBoB')" -o $S/v105.x86_64 && nm -u $S/v105.x86_64 | sort -u > $S/v105.u
nm -u _extracted/kext/M-AudioFireWireBeBoB.kext/Contents/MacOS/M-AudioFireWireBeBoB | sort -u > $S/v1112.u
cat $S/v19.u $S/v105.u $S/v1112.u | sort -u > $S/refs.u; comm -23 $S/ours.u $S/refs.u | c++filt

# E-3  10.13.6 exports (read-only, build PC)
ssh fw1814-build 'nm /System/Library/Extensions/System.kext/PlugIns/IOKit.kext/IOKit | grep IOLock'

# E-4  imports not defined in the 14.8.4 Intel kernel + families (KDK installed on the dev Mac)
KDK=/Library/Developer/KDKs/KDK_14.8.4_23J319.kdk
{ nm -arch x86_64 -U $KDK/System/Library/Kernels/kernel
  for k in IOAudioFamily IOFireWireFamily IOFireWireAVC; do nm -arch x86_64 -U "$(find $KDK/System/Library/Extensions/$k.kext.dSYM -path '*DWARF*' -type f)"; done
} | awk 'NF>=3{print $3}' | sort -u > $S/k14.def
comm -23 BROAD_COMPATIBILITY/evidence/kext-0.2.2-imports.txt $S/k14.def | c++filt

# E-5  arm64e families in the KDK
lipo -archs "$(find $KDK/System/Library/Extensions/IOFireWireFamily.kext.dSYM -path '*DWARF*' -type f)"
grep -o 'com.apple.iokit.IOFireWireFamily' $KDK/System/Library/KernelCollections/kernelcache.release.mac14g.manifest.plist

# E-10 panel demo run
"<OA BeBoB MAFW.app>/Contents/MacOS/OA BeBoB MAFW" --demo --snapshot $S/snap
```

## Limits of this evidence

* "Defined in the kernel" (E-4) is not "exported to kexts". The export check needs each OS's KPI symbol sets. On 10.13.6 they are files in `System.kext/PlugIns` (E-3). On macOS 11 and later they are not files on disk; use the matching KDK or a real `kextutil -n -t` / `kmutil` dry run on that OS (task BC-1.1).
* No 10.6.8 symbol set was read: the SL rig did not answer SSH on 2026-10-21. BC-1.1 still has to do it.
* E-5 proves that the kernel families exist on Apple Silicon in macOS 14.8.4. It does not prove that a FireWire device works there through the adapter chain.

## Added 2026-10-21 (R125)
| File | What it shows | Evidence id |
|---|---|---|
| `kext-0.2.2-kmutil-libraries-macos-14.8.9.txt` | `kmutil libraries` on the live 14.8.9 Mac: shipped 0.2.2 resolves 775/777 (only `_RESERVEDIOAudioEngine15/16` missing); the same sources built with the 15.2 SDK resolve 778/778 | E-17 |

## Added 2026-10-21 (R126): first load and play, Modern edition
Unlike the files above, this one comes from a real load on the dev Mac (the user installed with `openaudio-mafw-bebob/scripts/install-modern.sh`, approved the kext in Privacy & Security and restarted).

| File | What it shows | Evidence id |
|---|---|---|
| `modern-kext-first-run-audiophile-macos-14.8.9.txt` | The `BAND=modern` kext (UUID 20D9014D…) loaded on macOS 14.8.9 Intel; FW Audiophile attached (trace all `r=0x0`); CoreAudio device 4 in / 6 out; ~12 s playback heard by the user; TX callbacks 46/46 ms, no late gap, no CLIP jump; afterwards the FireWire controller went away and came back, and the driver re-attached without a panic | E-18 |

```sh
# E-18 (dev Mac, after install-modern.sh + approval + restart, interface on)
kmutil showloaded | grep -i -E 'openaudio|firewire|IOAudio'
ioreg -r -c com_openaudio_FWAudiophileDevice -w0 -l -d1 | grep '"MADebugTrace"' | tr ',' '\n'
system_profiler SPAudioDataType | grep -A8 'FireWire Audiophile'
say -a '?'                                    # device id of "FireWire Audiophile Multichannel" (87 on this run)
say -a 87 "[[volm 0.3]] Sound test ..."       # plays on channels 1-2 = analog out 1/2 (FWAudiophileTables.h:14)
clang -O2 -Wno-deprecated-declarations -o $S/maprop openaudio-mafw-bebob/tools/maprop.c -framework IOKit -framework CoreFoundation
MA_CLASS=com_openaudio_FWAudiophileDevice $S/maprop get MADebugTxStats   # counters reset at each TX Start (FWDCLProgram.cpp:579)
```

## Added 2026-10-21 (R125, R126) — used by plan v2 and v3

| File | What it shows | Evidence id |
|---|---|---|
| `kext-0.2.2-kmutil-libraries-macos-14.8.9.txt` | `kmutil libraries` on the live 14.8.9 collections: 775 of 777 imports resolve for the 10.13-SDK kext, 778 of 778 for the 15.2-SDK build | E-17 |
| `modern-kext-first-run-audiophile-macos-14.8.9.txt` | The Modern kext loads, attaches the FW Audiophile and plays; transmit timing | E-18 |
| `modern-kext-dropout-macos-14.8.9-1941.txt` | Logs of the one dropout (the adapter's FireWire controller leaves PCI); cause open | E-18, RK-6 |
