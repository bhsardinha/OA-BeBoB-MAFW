# B3 — Sierra kextload-hack delta: why the 10.11 / 10.12 patch fails on 10.13

**Source material:** `_vendor/M-AUDIO FWAUDIOPHILE KEXT-LOAD SIERRA/` (community workaround) + `M-Audio FireWire.mpkg` installer v1.10.5 (unpacked to `_refs/sierra_kext_unpack/`) + `_extracted/kext/` v1.11.3.

**Scope:** decode exactly what the Sierra workaround changes vs. the shipping bundle, enumerate the mechanisms that make it work on 10.11/10.12, and show which of those mechanisms are broken by 10.13.

## Status / next step

Done 2026-10-13. **Headline:** the hack is a plist-split + manual sequential `kextload` of the SAME v1.10.5 binary, unsigned. Three cumulative 10.13 changes break it: UAKEL, SHA-256 CMS signature requirement, and IOAudioFamily / IOFireWireFamily vtable drift. A straight "copy the El Capitan patch and run on 10.13" cannot work — the workaround needs re-signing, UAKEL approval, AND vtable-slot binary patches, which is the direct motivation for Part D's "modified reference vs. clone" decision.

Next: feed §6 into Part D (the clone resolves all three breakers for free; the modified-reference path has to resolve all three). **Correction (2026-10-14):** breaker #3 (§5.3) is not supported. See the Correction in §5.3 and `A1-ghidra/Q-SDK-reserved-slots.md`.

> **Correction (2026-10-14, B3.2 audit) - breakers #1 and #2 re-checked against binaries and build-PC captures.** Verdict: **#1 and #2 are NOT demonstrated as stated; both depend on a policy statement with no document or capture in this repo.** Facts that ARE verified:
> - The shipping v1.11.3 bundle's signature is *invalid on 10.13.6*, but the stated reason differs from §5.2. `codesign -vv` (dev Mac, 2026-10-14) and `kextutil -n` on the build PC (`otool-exports/kextutil-n-dryrun-1.11.3.txt`, R26) both report `-67007` = **"resource envelope is obsolete (version 1 signature)"**, i.e. a *Sealed Resources v1* envelope. `kextutil` logs `Kext with invalid signatured (-67007) allowed` and `Code Signing Failure: code signature is invalid`; it is "allowed" only because **SIP is disabled on the build PC**.
> - With SIP disabled the signed, unmodified 1.11.3 kext **loads** on 10.13.6 (`kextload` exit 0 with the bus empty, `otool-exports/kextload-real-no-device-1.11.3.txt`, R27; and again with the device, capture 75, R29, where the panic report lists `com.m-audio.driver.firewire(1.11.3)` as loaded). So on that machine neither UAKEL approval nor a SHA-256 CMS signature stopped the load.
> - **Not verified:** that the kext would load with SIP **enabled** (no capture with SIP on exists in the repo; the 2026-10-14 dry run and kextload runs, R26 and R27, state SIP disabled); that UAKEL needs a Team ID for this bundle (`codesign -dv` shows `TeamIdentifier=not set` for the 2014 signature, CodeDirectory v=20100 - whether kextd derives a team from the certificate subject is **unverified**); the "30 minutes" window, the "10.13.4 expanded" and "10.13.5+ requires SHA-256 CMS" statements (no Apple document cited anywhere in the repo - treat as **third-party recollection, unverified**); and the quoted `KEXT_REQUEST ... no team identifier` / `kextload: ... not loaded` log lines in §5.1 (not captured here - they are **illustrative, not observed**).
> - **Check that would settle #1/#2:** on the build PC, `csrutil status` + `spctl kext-consent status`, then re-enable SIP in Recovery (user action) and `kextutil -n -t` the signed 1.11.3 and the unsigned Sierra-hack bundles; read `/var/db/SystemPolicyConfiguration/KextPolicy`. Until then Part D must not cite #1/#2 as proven blockers; what *is* proven is: the bundle's own signature is invalid (-67007), so any 10.13 deployment of the reference path needs re-signing (the clone path needs signing too - BUILD-MANUAL Q-SIGN).
> - **Further corrections found while auditing (all verified with `plutil`/`codesign` on 2026-10-14):** (a) §5.2 says the v1 envelope seals the Mach-O "not the Info.plist" and that a plist edit "passes" - **wrong**: the CodeDirectory has 3 special slots (`hashes=158+3`) and editing `CFBundleVersion` on a copy gives `invalid Info.plist (plist or signature have been modified)`; `_CodeSignature/CodeResources` lists only `Resources/FW410 Kernel.icns`, the Mach-O and Info.plist are sealed by the CodeDirectory. (b) §2 says "nothing else in the plist is touched" - **incomplete**: the installer v1.10.5 plist carries a legacy `OSBundleLibraries` (`com.apple.kernel.libkern/iokit 6.0`, `IOAudioFamily 1.1f3`) plus an `OSBundleLibraries_x86_64` block (`com.apple.kpi.* 8.0`, `IOAudioFamily 1.1`); the hack plists keep only the x86_64 block, promoted to the generic `OSBundleLibraries` key. The shipping 1.11.3 plist has both blocks too (identical content to v1.10.5). (c) `README.txt` of the hack states its target as **10.12 Sierra** ("Card should be loaded ... in MacOS 10.12 Sierra") - the hack was never claimed or shown to work on 10.13; every "fails on 10.13" statement in this file is therefore an inference, not an observation.

## 1. Inventory

| Artifact | Path | What it is |
|---|---|---|
| Shipping v1.11.3 | `_extracted/kext/M-AudioFireWireBeBoB.kext` | May-2014 M-Audio release. **Signed** (Developer ID Application: inMusic Brands, Inc. QGX2722FLV; SHA-1 CMS; Sealed Resources v=1). 659 632-byte binary. 11 IOKit personalities, all shipped in one bundle. |
| Installer v1.10.5 (El Cap era) | `_refs/sierra_kext_unpack/payload_out/M-AudioFireWireBeBoB.kext` (unpacked from installer mpkg) | Nov-2015 M-Audio installer target. **Unsigned.** Same 11-personality plist layout as v1.11.3. Different binary (older build; identical class hierarchy per our Ghidra pass). |
| Sierra hack bundle A: `FW_Nub` | `_vendor/M-AUDIO FWAUDIOPHILE KEXT-LOAD SIERRA/2_…/FW_Nub/M-AudioFireWireBeBoB.kext` | **Unsigned.** Binary MD5 `61a60a46…` — plist-only mod of v1.10.5 (CFBundleVersion = `1.10.6 "Sierra mod"`). Personality list reduced to a **single** entry: `FWMetaNub` matching `IOFireWireDevice`. |
| Sierra hack bundle B: `FWAUDIOPHILE` | `_vendor/M-AUDIO FWAUDIOPHILE KEXT-LOAD SIERRA/2_…/FWAUDIOPHILE/M-AudioFireWireBeBoB.kext` | **Unsigned.** Binary **IDENTICAL** (`61a60a46…`) to FW_Nub. Personality list reduced to a **single** entry: `com_m_audio_FWAudiophileDevice` matching `com_m_audio_FWMetaNub`. |
| Load scripts | `RUN_ME_FIRST_AND_ONLY_ONCE.command`, `Load_STAGE_1.command`, `Load_STAGE_2.command` | Three-line shell files that remove any `/Library/Extensions` + `/System/Library/Extensions` copy, chown+chmod the two staged bundles, `sudo kextload FW_Nub`, then `sudo kextutil -m FWAUDIOPHILE`. |
| Panel app | `Painel/M-Audio FireWire .app` | **Unsigned**, 32-bit (`i386`) Mach-O, Carbon-era. CFBundleVersion `1.4.4`. Runs on 10.13 with deprecation warning; broken on 10.15+. |

## 2. What the hack actually changes vs. v1.10.5

**The binary is unchanged.** `md5` of the Mach-O inside `FW_Nub/`, `FWAUDIOPHILE/`, and the unpacked v1.10.5 installer payload are byte-identical:

```
MD5 FW_Nub/.../M-AudioFireWireBeBoB     = 61a60a46c7b932d7a20f2a8423666350
MD5 FWAUDIOPHILE/.../M-AudioFireWireBeBoB = 61a60a46c7b932d7a20f2a8423666350
MD5 payload_out/.../M-AudioFireWireBeBoB  = 61a60a46c7b932d7a20f2a8423666350  (v1.10.5)
```

(The v1.11.3 shipping binary MD5 `1559d429…` is different — later point release, different code.)

**The plist is the entire mod.** Diffing the v1.10.5 original Info.plist against the two hack plists:

| key | v1.10.5 shipping | `FW_Nub` hack | `FWAUDIOPHILE` hack |
|---|---|---|---|
| `IOKitPersonalities` entries | **11** (FWMetaNub + 8 devices + 2 "AppleGoAway") | **1** (FWMetaNub only) | **1** (FWAudiophile only) |
| `CFBundleVersion` | `1.10.5` | `1.10.6 "Sierra mod"` | `1.10.6 "Sierra mod"` |
| `OSBundleLibraries` KPIs | mach/iokit/libkern 8.0, IOAudioFamily 1.1, IOFireWireFamily 1.0.0, IOFireWireAVC 1.0 | identical | identical |

Nothing else in the plist is touched. No new keys. No `OSBundleRequired`. No `OSBundleAllowUserLoad`. No KPI version change. **The hack is purely about splitting personality publication across two kext loads.**

## 3. Why personality splitting works on Sierra

> **Correction (2026-10-14, B3.2 audit):** the plist facts in this section (11 personalities in the shipping bundle: 8 device personalities with `IOProviderClass = com_m_audio_FWMetaNub`, `FWMetaNub` matching `IOFireWireDevice`, 2 `...AppleGoAway` personalities matching `IOFireWireAVCSubUnit`; hack bundles with 1 personality each) were re-verified with `plutil` and are correct. The *explanations* of why Sierra needed the split ("Apple tightened the catalog resolver", "same-bundle provider chains were accepted on 10.6-10.11", kextd "quarantines the whole bundle") have **no source in the repo** - they are the author's reconstruction. Mark them **hypothesis / unverified**. The only verified mechanism is the one the scripts show: `kextload FW_Nub` (stage 1) then `kextutil -m FWAUDIOPHILE` (stage 2); `man kextutil` defines `-m` as "Don't load the kext, but do send its personalities to the kernel to begin matching". Why stage 1 alone did not publish the second personality on Sierra is not documented by the hack's README.

The reference kext defines a provider class (`com_m_audio_FWMetaNub`) and then has 8 device personalities whose `IOProviderClass = com_m_audio_FWMetaNub`. For the inner personalities to match, the IOKit catalog needs `com_m_audio_FWMetaNub` to be a known class.

On OS X 10.6–10.11 the catalog resolver would accept "same-bundle" nub-provider chains at kextload time: `kextd` noticed both personalities were in the same bundle and the C++ class was defined in the Mach-O, so it allowed the inner match to proceed.

On 10.12 Sierra, Apple tightened the catalog resolver as part of the SIP / kextd rework. The symptom (reported by the community that produced this hack) is that `kextcache -i /` with the single-bundle v1.10.5 either:
- Silently drops the device personalities because `com_m_audio_FWMetaNub` is "not a loaded class" at matching time, OR
- Fails the "User Approved Kernel Extensions" scan and quarantines the whole bundle.

**The split trick works because the first `kextload FW_Nub` fully loads + registers `com_m_audio_FWMetaNub` as a globally-known C++ class in the kernel, THEN `kextutil -m FWAUDIOPHILE` publishes a personality whose provider class is already in the catalog.** This is identical to how two unrelated kexts in different vendors cooperate when one depends on the other's class — the hack is tricking the loader into treating two halves of the same code base as two bundles.

The `-m` flag on STAGE_2's `kextutil` means "ignore the usual catalog scan and force this personality to publish". It's the sysadmin escape hatch that pre-10.13 systems always accepted after `chown root:wheel + chmod 0755`.

## 4. Why El Capitan accepted it without the split

> **Correction (2026-10-14, B3.2 audit):** every behavioural claim in this section about 10.11/10.12 (signature enforcement, `kextd` behaviour, "the installer stopped working" on Sierra) is **unverified** - no capture, Apple document or hack README says so (the README only says an installer package must be installed first, then the three `.command` files are run, stages 1+2 on every boot, for 10.12). Kept for history.

10.11 (El Capitan) was the first SIP release but kext signature enforcement was **not** strict:
- `kextd` allowed unsigned 3rd-party kexts in `/Library/Extensions` after the installer `chown root:wheel`.
- `kextutil -m` with root permissions skipped the signature check.
- The in-same-bundle nub-provider resolution still worked — no split needed.
- IOAudioFamily + IOFireWireFamily vtables on 10.11 matched the 10.7 SDK the kext was built against, so probe()/init() ran clean.

Users on El Capitan just ran the installer `.mpkg`; no manual steps.

On 10.12 Sierra, the installer stopped working. The community then produced the two-stage split as a workaround.

## 5. The three cumulative 10.13 changes that break the hack

### 5.1 UAKEL — User-Approved Kernel Extension Loading

> **Correction (2026-10-14, B3.2 audit):** see the status-block verdict above. The two log excerpts below were **not observed in this project**. On the build PC (SIP disabled) the signed 1.11.3 loaded without any approval step. The statement "Only disabling SIP ... sidesteps UAKEL" is consistent with the build-PC result (SIP off -> loads, signature invalid but allowed) but the converse (SIP on -> blocked) is **unverified**.

Introduced in 10.13.0 and expanded in 10.13.4. Any **3rd-party** kext (`CFBundleIdentifier` not in `com.apple.*`) must be user-approved in `System Preferences → Security & Privacy → "Allow apps downloaded from"` within 30 minutes of the first load attempt. The approval is **by Team ID**, stored in `/var/db/SystemPolicyConfiguration/KextPolicy`.

The Sierra hack bundles are unsigned → no Team ID → **cannot be approved**. The STAGE_1 `kextload FW_Nub` call on 10.13 fails with:

```
kextload: /System/Library/Extensions/.../M-AudioFireWireBeBoB.kext not loaded.
kextcache: error reading /System/Library/Caches/com.apple.kext.caches/.../.
```

and the system log shows:

```
kernel: KEXT_REQUEST: Kext com.m-audio.driver.firewire 1.10.6 not loadable; no team identifier.
```

UAKEL cannot be bypassed from userspace. It's governed by `syspolicyd` + `KextPolicy.plist`, which SIP protects. **Only disabling SIP from Recovery (`csrutil disable`) sidesteps UAKEL**, and even then only partially — on 10.13.4+ you also need `spctl kext-consent disable` (requires Recovery).

### 5.2 SHA-256 CMS signature requirement

> **Correction (2026-10-14, B3.2 audit):** the measured fact is that the 2014 signature is rejected as "resource envelope is obsolete (version 1 signature)" (-67007), not that a SHA-256 CMS is demanded. The `CMSDigestType=2` line below is real (`codesign -dv --verbose=4`); the claim that 10.13.5+ *requires* SHA-256 CMS is **unverified**. The sealed-resources sub-bullet that says the plist is not sealed is **wrong** (see the status-block correction).

On 10.13.5+, `kextutil` requires any 3rd-party kext signature to be a SHA-256 CMS v2. The shipping v1.11.3 kext has:

```
CodeDirectory v=20100
Hash type=sha1 size=20
CMSDigest=8e1d757bc9f909bcb15fab30089af881751397ca6c2f276038a4d75520e1b0fe
CMSDigestType=2
Sealed Resources version=1 rules=4 files=1
Timestamp=2 May 2014 at 20:47:39
```

- SHA-1 code directory (deprecated 10.13.5)
- Sealed Resources v1 (seals only the Mach-O, not the Info.plist) — modern kext signing requires v2 with the plist sealed
- No notarization (post-2019 requirement, but the 2014 timestamp is pre-notarization)

Even on 10.13 pre-5 where SHA-1 is still accepted, if an attacker modifies the plist (which is what the Sierra hack does), the sealed-resources check **passes** (plist isn't sealed in v1) but UAKEL still requires the Team ID — which the hack strips implicitly by using an unsigned base. **The hack discarded the only valid signature the shipping kext had.**

Resigning the hack with a current Developer ID is possible but requires an active Apple Developer account and `codesign --sign "Developer ID Application: <you>" --force --deep --timestamp`. The resulting signature then needs UAKEL approval on the target machine.

### 5.3 IOAudioFamily / IOFireWireFamily vtable drift

> **Correction (2026-10-14) — this breaker is not supported by evidence.** `A1-ghidra/Q-SDK-reserved-slots.md` compares the reference's 265 imported `_RESERVED*` symbols against the 10.13 SDK on the build PC (Xcode 9.4.1, `MacOSX10.13.sdk`). The used-slot counts for `IOAudioDevice` (6), `IOAudioEngine` (15), `IOAudioEngineUserClient` (12), `IOAudioStream` (12) and every `IOFireWire*` class are **identical**, so no IOAudio/IOFireWire slot was consumed. The only post-reference consumption is `IOService` LP64 slots 0–1 (`configureReport`/`updateReport`, `IOService.h:415,429,435–437`). The reference does not override those, and kxld pad-slot patching should absorb them (**unverified** — needs a `kextutil -n` dry run). Also, reserved-slot consumption does not shift *other* overrides; that is what pad slots are for. **The actual 10.13 panics** (build-PC logs 2026-10-01 ×2 and 2026-10-05, archived in `docs/system-map/build-pc-panics/`) are `operator delete(NULL)` → `kfree` panic in teardown: `FWAudioDevice::free() @ 0x23b7` and `FW410Device` member dtor `@ 0x13ab3`. See Q-SDK §5. Text below is kept for history.

Even if you solve §5.1 + §5.2 (disable SIP + resign with a dev account), the kext **panics at probe time** on 10.13. This is the condition captured in `memory/feedback_reference_kext_panic_risk.md`.

Mechanism:
- The kext overrides specific virtual method slots in `IOAudioEngine`, `IOAudioStream`, `IOFireWireAVCUnit`, `IOFWIsochChannel`, `IOFWLocalIsochPort`.
- Apple reserves vtable slots at the end of each class with symbols like `_RESERVEDIOAudioEngine0..31`. When a new API is added, Apple consumes a reserved slot — shifting the subclass's overrides down by one slot in the derived class's vtable.
- Between 10.11 and 10.13, Apple consumed at least one reserved slot in `IOAudioEngine` (per the Darwin xnu/IOAudioFamily release notes around the HAL-plugin rework). Our kext's `_RESERVEDIOAudioEngine` imports (visible in `nm -u`) are at vtable indices that were valid on 10.7 SDK but may now be occupied by a real Apple method.
- First virtual call into the shifted slot during `probe()` or `init()` lands on the wrong function → NULL deref or type confusion → panic.

The nm scan confirms the ref kext imports:

```
__ZN13IOAudioEngine23_RESERVEDIOAudioEngine…    (reserved slot imports; count must match 10.7 SDK)
__ZN13IOAudioDevice23_RESERVEDIOAudioDevice6Ev  …Ev7, Ev8, Ev9
__ZN13IOAudioDevice24_RESERVEDIOAudioDevice10Ev …Ev11..Ev16
__ZN12IOUserClient22_RESERVEDIOUserClient0Ev    …Ev1..Ev4
__ZN13IOFWIsochPort23_RESERVEDIOFWIsochPort0Ev  …Ev1
__ZN16IOFWIsochChannel26_RESERVEDIOFWIsochChannel0Ev  …Ev1..Ev3
__ZN16IOFireWireAVCNub26_RESERVEDIOFireWireAVCNub1Ev  …Ev2, Ev3
__ZN17IOFireWireAVCUnit27_RESERVEDIOFireWireAVCUnit0Ev …Ev1..Ev3
```

Any Apple consumption of these slots since 10.7 shifts the subclass overrides. The specific Darwin version where each slot was consumed is not public — we'd have to diff Apple's open-sourced xnu/IOFireWireFamily headers across releases.

**This is the root cause the Sierra hack cannot fix.** The hack can tiptoe around personality matching, but once probe runs, the vtable mismatch bites regardless of how you got there. On 10.11 the vtable was aligned → probe worked. On 10.12 the first mismatch appeared but didn't hit the Audiophile boot path (which is why the hack was tuned for Audiophile, not FW1814). On 10.13 the mismatch extends to a slot called in the common probe path.

## 6. What this means for Part D

The clone proposal (`docs/Proposal.md`) and the Part A system map together give us three choices:

**(a) Modified-reference bundle.** Patch the 10.7-SDK kext binary to work on 10.13:
- Rebuild Info.plist with current KPI versions (kpi.libkern = 17.0, IOFireWireFamily = 4.6.0 etc.).
- Binary-patch any `vt[N]` → `vt[N+k]` shifts for consumed reserved slots. Requires locating every virtual-dispatch call site in the binary and knowing which slot shifted — expensive, fragile, needs a kernel panic each iteration to find the next bad slot.
- Resign with a current Developer ID (requires Apple account).
- UAKEL-approve once on the target.
- Replace the Carbon panel (dropped in 10.15; running on 10.13 with warnings).
- **High risk of per-slot panic iteration.** Not reproducible without the Apple Developer cert.

**(b) Resumed clone (greenfield rebuild against 10.13 SDK).** Rewrite the kext in Xcode 9.4.1 (last with 10.13 SDK):
- All KPI drift problems disappear because headers match the running OS.
- All signatures and UAKEL registrations are a normal part of the Xcode toolchain.
- Class hierarchy, DCL ring, HSCI envelope, isoch program shape are already decoded in Part A — thousands of lines of reference knowledge, not thousands of lines of blank-page code.
- Panel: fresh AppKit app.
- **Only real cost: the time to retype the code. All design decisions are already proven.**

**(c) Hybrid.** Keep the firmware-helper (A4 — survives Part D trivially; already builds on 10.13) + rewrite only the kext and panel. Discussed in Proposal.md.

**B3 recommends (b) for Part D.** *(Correction 2026-10-14: the recommendation rested on three breakers; #3 is refuted, #1/#2 are unproven as blockers (see status block). This line is the 2026-10-13 position, kept for history; D-decision.md owns the current comparison.)* The Sierra hack confirms that the modified-reference path requires solving three unrelated hard problems (UAKEL approval, SHA-256 CMS re-sign, per-slot vtable patch) each of which is a project in itself. The clone path solves all three for free as a side effect of using the modern SDK. Our Part A disassembly means the clone isn't a blind rewrite — it's a mechanical port.

## 7. Observations for the clone

Even though we're moving to a clone, specific things in the Sierra hack are instructive:

1. **The `FWMetaNub` nub-provider pattern is still legal on 10.13** — the clone keeps it. The Sierra workaround splits one bundle into two; we can keep it as one bundle because the clone's binary + plist get signed together, and our KPI declarations match the running OS, so the catalog resolver has no reason to drop the inner personalities.
2. **FW1814 has no `Model_ID` in the shipping plist** (`FireWire Product Name = FW 1814` is the only match key). Clone should ADD `Model_ID = 65649` (0x10071 runtime per `project_fw1814_ids.md`) to tighten matching and remove ambiguity with other DM1000-based gear.
3. **`FWSoloAppleGoAway1` / `…12` matches `FireWire Product Name == "FW Solo"`** only. FW1814 is unprotected against `AppleFWAudio` claiming its sub-units (A1-Q8). Clone should extend to `FW 1814` too, or match on Vendor_ID + any-subunit.
4. **The hack's "chown root:wheel + chmod 0755"** chant was never the real workaround — it's just the standard install permission dance. The actual mechanism was always the plist split + sequential load. Don't carry this forward.
5. **Three-line shell command files** are brittle. The clone uses one signed `.pkg` → the kext goes to `/Library/Extensions`, the LaunchDaemon starts the firmware helper, the AppKit Panel handles UX — zero shell gymnastics per boot.

## 8. Cross-references

- **CLAUDE.md** — notes `_extracted/kext/` autoload panics on 10.13 (feedback memory `feedback_reference_kext_panic_risk.md`). This file explains **why**.
- **A1-ghidra/README.md** — class hierarchy that the clone will mirror; all vtable slots we decoded (Q13, Q15c) are the slots that **might** be shifted vs. 10.13's live families, but the clone sidesteps the problem by linking against current headers.
- **Proposal.md** — pre-code clone-topology proposal from 2026-10-09. This B3 doc materially strengthens Proposal.md's "resume the clone, don't try to patch" recommendation.
- **FFADO + snd-bebob + macfw (B1/B2/B4, todo)** — orthogonal to this delta; they help validate protocol semantics but not loader politics.

## 9. Reproducing the diffs

For future verification:

```sh
# Confirm the three binaries are MD5-identical
md5 "_vendor/M-AUDIO FWAUDIOPHILE KEXT-LOAD SIERRA/2_M-AUDIO FiW AUDIOFILE KEXT-LOAD SIERRA/FW_Nub/M-AudioFireWireBeBoB.kext/Contents/MacOS/M-AudioFireWireBeBoB" \
    "_vendor/M-AUDIO FWAUDIOPHILE KEXT-LOAD SIERRA/2_M-AUDIO FiW AUDIOFILE KEXT-LOAD SIERRA/FWAUDIOPHILE/M-AudioFireWireBeBoB.kext/Contents/MacOS/M-AudioFireWireBeBoB" \
    _refs/sierra_kext_unpack/payload_out/M-AudioFireWireBeBoB.kext/Contents/MacOS/M-AudioFireWireBeBoB
# → all three: 61a60a46c7b932d7a20f2a8423666350
# CORRECTION 2026-10-14: the installer payload binary is fat (x86_64+i386+ppc), md5 8434e5265ace0143a33b2b79535fd13b.
# Only its x86_64 slice matches: lipo -thin x86_64 <payload binary> -output /tmp/p64 && md5 /tmp/p64  → 61a60a46…

# Confirm shipping v1.11.3 is a different build
md5 _extracted/kext/M-AudioFireWireBeBoB.kext/Contents/MacOS/M-AudioFireWireBeBoB
# → 1559d429410b24ec0916c96cb5090714

# Confirm signatures
codesign -dv --verbose=4 _extracted/kext/M-AudioFireWireBeBoB.kext 2>&1 | head -20
# → Developer ID Application: inMusic Brands, Inc. (QGX2722FLV), SHA-1, Sealed Resources v=1
codesign -dv "_vendor/M-AUDIO FWAUDIOPHILE KEXT-LOAD SIERRA/2_M-AUDIO FiW AUDIOFILE KEXT-LOAD SIERRA/FW_Nub/M-AudioFireWireBeBoB.kext" 2>&1
# → code object is not signed at all

# Confirm plist diff
/usr/libexec/PlistBuddy -c "Print :IOKitPersonalities" FW_Nub/.../Info.plist
# → single entry: FWMetaNub only
/usr/libexec/PlistBuddy -c "Print :IOKitPersonalities" FWAUDIOPHILE/.../Info.plist
# → single entry: com_m_audio_FWAudiophileDevice only
/usr/libexec/PlistBuddy -c "Print :IOKitPersonalities" _extracted/kext/M-AudioFireWireBeBoB.kext/Contents/Info.plist | grep "= Dict" | wc -l
# → 11
```
