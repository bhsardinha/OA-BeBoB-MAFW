#!/usr/bin/env python3
"""Panel <-> kext <-> plist conformance (BUILD-PLAN §0, Build row IN.7). Static: reads the sources, needs no hardware or toolchain.

Checks (each prints OK/FAIL, exit status 1 if any FAIL):
  1. every MA* key of the panel catalog is registered by the kext, with a compatible kind (array / number / 64-bit / function)
  2. array element counts of the panel catalog equal the kext's counts (group templates in FWAudioDeviceChain.h, counts in the AddArray calls)
  3. the panel's match class equals the personality IOClass, the personality carries IOUserClientClass, the helper matches the nub class
  4. the panel's meter-page map (52 slots = 18 + 20 + 14) equals the kext's peak constants and the user-client memory type
  5. the panel's per-rate channel counts equal the kext's DeviceChannels tables (FWRates.h)
  6. CoreAudio controls are created by FWSettingsLevels::Init
Usage: scripts/conformance_check.py   (from anywhere; paths are relative to this tree (openaudio-mafw-bebob/))
"""
import glob, os, re, sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
def read(p): return open(os.path.join(ROOT, p)).read()
fails = 0
def check(ok, msg):
    global fails
    print(("OK   " if ok else "FAIL ") + msg)
    if not ok: fails += 1

# ---- kext side ---------------------------------------------------------------------------------------------------------------------
mixer_h = read("kext/src/models/FW1814/FW1814Device.h")       # the FW1814 device owns the groups (AP-2.2, module models/FW1814)
cnt = {}
for n, names in re.findall(r"FWConcreteChannelGroup<(\d+)>\s+([^;]+);", mixer_h):
    for nm in names.split(","): cnt[nm.strip()] = int(n)
group_count = {"SWReturn": cnt["fSWReturn"], "Output": cnt["fOutput"], "Input": cnt["fInput"], "Headphone": cnt["fHeadphone"],
               "AuxOutput": cnt["fAuxOut"], "AuxSend": cnt["fAuxSend"]}
sym = {"swReturn": cnt["fSWReturn"], "output": cnt["fOutput"], "input": cnt["fInput"], "headphone": cnt["fHeadphone"],
       "auxOut": cnt["fAuxOut"], "auxSend": cnt["fAuxSend"]}
src = "".join(read(os.path.relpath(p, ROOT)) for p in sorted(glob.glob(os.path.join(ROOT, "kext/src/**/*.cpp"), recursive=True)))
mixer_cpp = read("kext/src/models/FW1814/FW1814Device.cpp")
n_monitor = (cnt["fOutput"] >> 1) + 1                     # FW1814Device::InitializeUserSettableProperties: fAux->Init(..., (fOutput.Count() >> 1) + 1)
check("(fOutput.Count() >> 1) + 1" in mixer_cpp, "kext: nMonitor expression is (output.Count() >> 1) + 1 = %d" % n_monitor)

def eval_count(expr):
    e = expr.strip()
    if e == "nMonitor": return n_monitor
    m = re.match(r"(\w+)->Count\(\)(?:\s*>>\s*(\d+))?$", e)
    if m: return sym[m.group(1)] >> int(m.group(2) or 0)
    return int(e)

kext = {}   # key -> (kind, count)
for key, expr in re.findall(r'AddArray\("(MA\w+)",\s*([^,]+?),', src): kext[key] = ("array", eval_count(expr))
for key in re.findall(r'AddNumber32\("(MA\w+)"', src): kext[key] = ("number32", None)
for key in re.findall(r'AddNumber64\("(MA\w+)"', src): kext[key] = ("number64", None)
for key in re.findall(r'AddFunction\("(MA\w+)"', src): kext[key] = ("function", None)
for key, slotvar in re.findall(r'AddChannelGroup\("(MA\w+)",\s*(\w+),', src):
    kext[key] = ("array", sym[slotvar])
for name in re.findall(r'InitGroupProp\("(\w+)"', src):                   # MA<group>{Levels,Mute,Solo,Link}
    for suffix in ("Levels", "Mute", "Link") + (("Solo",) if name != "Headphone" else ()):
        kext["MA%s%s" % (name, suffix)] = ("array", group_count[name])
literals = set(re.findall(r'"(MA[A-Za-z0-9]+)"', src))

# ---- panel side --------------------------------------------------------------------------------------------------------------------
keys_swift = read("control-panel/Sources/MaKeys.swift")
panel = {}
for name, kind, count, access in re.findall(r'MAKey\(name: "(MA\w+)",\s*kind: \.(\w+)(?:\(count: (\d+)\))?,\s*access: \.(\w+)', keys_swift):
    panel[name] = (kind, int(count) if count else None, access)
compat = {"levels": "array", "boolArray": "array", "routing": "array", "scalar32": "number32", "mask64": "number64", "function": "function"}
for name, (kind, count, access) in sorted(panel.items()):
    if kind in ("string", "dictionary"):
        check(name in literals, "%-30s descriptor/read-only key published by the kext" % name); continue
    if name in ("MAMetaAllProperties",): continue
    if name not in kext:
        if name == "MASampleRate" and 'd->getObject("MASampleRate")' in src:
            check(True, "%-30s handled by the setProperties fast path (applied before the dictionary dispatch)" % name)
        elif name in literals and access in ("readOnly", "writeOnly"):
            check(True, "%-30s published via setProperty / handled by the nub (panel %s, %s)" % (name, kind, access))
        else:
            check(False, "%-30s missing in the kext (panel kind %s)" % (name, kind))
        continue
    kk, kc = kext[name]
    ok = (kk == compat[kind]) or (kk == "number32" and kind == "scalar32")
    if kk == "array" and count is not None: ok = ok and kc == count
    check(ok, "%-30s panel %s%s  kext %s%s" % (name, kind, "(%d)" % count if count else "", kk, "(%d)" % kc if kc else ""))
extra = sorted(k for k in kext if k not in panel)
check(not extra, "kext keys unknown to the panel catalog: %s" % (", ".join(extra) or "none"))

# ---- plist, class names, helper ---------------------------------------------------------------------------------------------------
plist = read("kext/Info.plist")
panel_class = re.search(r'kFWDeviceClassName = "(\w+)"', read("control-panel/Sources/IOKitBridge.swift")).group(1)
# generic first (plan v4): panel and helper match a generic base class; it must be an ancestor of every device personality's IOClass
hdrs = "".join(read(os.path.relpath(p, ROOT)) for p in sorted(glob.glob(os.path.join(ROOT, "kext/src/**/*.h"), recursive=True)))
parent = dict(re.findall(r"^class (\w+)\s*:\s*public (\w+)", hdrs, re.M))
def ancestors(c):
    out = []
    while c in parent: c = parent[c]; out.append(c)
    return out
dev_classes = [c for c in re.findall(r"<key>IOClass</key>\s*<string>(\w+)</string>", plist) if "com_openaudio_FWAudioDevice" in ancestors(c)]
helper_dev = re.search(r'kDeviceClass = "(\w+)"', read("firmware-helper/main.c")).group(1)
check(bool(dev_classes) and all(panel_class in ancestors(c) for c in dev_classes), "panel match class %s is a base of every device personality (%s)" % (panel_class, ", ".join(dev_classes)))
check(all(helper_dev in ancestors(c) for c in dev_classes), "helper device class %s is a base of every device personality" % helper_dev)
m = re.search(r"<key>FW1814Device</key>.*?</dict>", plist, re.S)
check(m is not None and "IOUserClientClass" in m.group(0), "FW1814Device personality carries IOUserClientClass")
helper_class = re.search(r'kNubClass = "(\w+)"', read("firmware-helper/main.c")).group(1)
check(re.search(r"<key>IOClass</key>\s*<string>%s</string>" % helper_class, plist) is not None, "helper nub class %s is a personality IOClass" % helper_class)
for key in ("MAFirmwareStart", "MAFirmwareInfo"):
    check(key in literals, "helper/panel key %s exists in the kext" % key)

# ---- meter page --------------------------------------------------------------------------------------------------------------------
chain_h = read("kext/src/base/FWAudioDevice.h") + read("kext/src/models/FW1814/FW1814Device.h")
peak_count = int(re.search(r"GetPeakLevelCount\(\) const \{ return (0x[0-9a-f]+)", chain_h).group(1), 16)
sw_first = int(re.search(r"GetFirstSWReturnPeakMeterIndex\(\) const APPLE_KEXT_OVERRIDE \{ return (0x[0-9a-f]+)", chain_h).group(1), 16)   # FW1814 override
sw_count = int(re.search(r"GetSWReturnPeakMeterCount\(\) const APPLE_KEXT_OVERRIDE \{ return (0x[0-9a-f]+)", chain_h).group(1), 16)
bridge = read("control-panel/Sources/IOKitBridge.swift")
panel_peak = int(re.search(r"static let peakCount = (\d+)", bridge).group(1))
# AP-5.1: the panel's meter map is the generic translator over each model profile's counts (control-panel/Sources/Translator.swift):
# SW returns start at inputs + outputs, outputs at inputs. Cross-check every model profile with the kext's per-model first SW-return slot.
def profile_counts(path):
    m = re.search(r"TranslatorCounts\(outputs: (\d+), mixer: (\d+), inputs: (\d+), headphone: (\d+)\)", read(path))
    return tuple(int(x) for x in m.groups())
tr = read("control-panel/Sources/Translator.swift")
check("func swReturn(_ k: Int) -> Int { return counts.inputs + (swReturnsFirst ? 0 : counts.outputs) + k - 1 }" in tr and "func output(_ id: Int) -> Int { return counts.inputs + (swReturnsFirst ? swReturnCount : 0) + id - 1 }" in tr,
      "panel translator: output slot = inputs + id - 1, SW-return slot = inputs + outputs + k - 1 (A2-P13 §6, PNAP §6); FW410 order [inputs | SW returns | outputs]")
o14, _, i14, _ = profile_counts("control-panel/Sources/Models/FW1814/FW1814Profile.swift")
check(i14 + o14 == sw_first and sw_first + sw_count == peak_count and i14 == cnt["fInput"],
      "FW1814: panel inputs %d + outputs %d = kext first SW-return slot %d; %d SW slots to the end of the page; inputs = kext input group" % (i14, o14, sw_first, sw_count))
uc = read("kext/src/base/FWUserClient.cpp")
check("type == 1" in uc and "meterMemory" in uc, "user client serves the meter page as clientMemoryForType(1); panel maps type 1")
check("IOConnectMapMemory64(connection, 1," in bridge, "panel maps memory type 1")

# ---- per-rate channel counts ---------------------------------------------------------------------------------------------------------
rates_h = read("kext/src/models/FW1814/FW1814Tables.h")
tables = {n: (int(i), int(o)) for n, i, o in re.findall(r"FWDeviceChannels kFW1814(\w+)\s*=\s*FWDC_HEAD\((\d+),\s*(\d+)\)", rates_h)}
analog_in, analog_out, spdif, adat = 8, 4, 2, 8
exp = {   # (kext table, panel expectation from ChannelLayout: analog + digital)
    "SPDIF44_96": (analog_in + spdif, analog_out + spdif),
    "ADAT44_48": (analog_in + adat, analog_out + adat),
    "ADAT88_96": (analog_in + adat // 2, analog_out + adat // 2),
    "_176_192": (2, 4),
}
for t, (i, o) in exp.items():
    check(tables.get(t) == (i, o), "FWRates %-14s in/out kext %s, panel layout %s" % (t, tables.get(t), (i, o)))
layout = read("control-panel/Sources/Models/FW1814/FW1814Layout.swift")          # the FW1814 rules moved to its panel module (AP-5.1)
check("rateClass == .quad { return s }" in layout and "adatHalf" in layout, "ChannelLayout implements the 176.4/192 kHz analog-only and the 88.2/96 kHz ADAT-half rules")

# ---- CoreAudio controls -------------------------------------------------------------------------------------------------------------
settings = read("kext/src/base/FWSettings.cpp")
for needle in ("EstablishVolumeControl(eng, kOutp, 0, 0", "EstablishVolumeControls(eng, kOutp, 1", "EstablishMuteControls(eng, kOutp, 1",
               "EstablishVolumeControls(eng, kInpt, 1", "EstablishMuteControls(eng, kInpt, 1", "EstablishMuteControl(eng, kIOAudioControlUsageOutput, 0, 0)"):
    check(needle in settings, "FWSettingsLevels::Init creates: %s" % needle)

# ---- device table vs the reference binary (AP-2.1; Q-AP0 §1.1, R77) ----------------------------------------------------------------
# kDeviceSpecs @0x46450 of the v1.11.2 x86_64 slice: 8 x {char *runtime, char *bootloader, char *class}; file offset = VM address.
slice = os.path.join(ROOT, "..", "_extracted", "kext", "M-AudioFireWireBeBoB.x86_64-slice")
rows = re.findall(r'\{\s*"([^"]+)",\s*"([^"]+)",\s*"([^"]+)"\s*\}', read("kext/src/base/FWMetaNub.cpp").split("kDeviceSpecs[] = {")[1].split("};")[0])
if os.path.exists(slice):
    import struct
    b = open(slice, "rb").read()
    cstr = lambda a: b[a:b.index(b"\0", a)].decode()
    ref = [tuple(cstr(p) for p in struct.unpack_from("<QQQ", b, 0x46450 + 24 * i)) for i in range(8)]
    check(rows == ref, "kDeviceSpecs equals the reference table @0x46450 (%d rows)" % len(rows))
else:
    print("SKIP kDeviceSpecs vs reference: _extracted/ not present (private repository only)")
check(len(rows) == 8 and ("FW 1814", "FW 1814 Bootloader", "com_m_audio_FW1814Device") in rows and
      ("FW Audiophile", "FW Audiophile Bootloader", "com_m_audio_FWAudiophileDevice") in rows, "kDeviceSpecs has 8 rows incl. FW 1814 and FW Audiophile")

# ---- FireWire Audiophile module (AP-3.1; Q-AP0 §2.4, §6, Q-AP0b §2) ---------------------------------------------------------------
ap_dir = os.path.join(ROOT, "kext/src/models/FWAudiophile")
if os.path.isdir(ap_dir):
    ap_t = read("kext/src/models/FWAudiophile/FWAudiophileTables.h"); ap_h = read("kext/src/models/FWAudiophile/FWAudiophileDevice.h")
    ap_rates = [int(x) for x in re.findall(r"\{ (\d+), 0 \}", ap_t.split("kFWAudiophileRates[5] = {")[1].split("};")[0])]
    m = re.search(r"kFWAudiophileChannels = \{\s*(\d+), (\d+), (\d+), (\d+), (\d+), 0, \{ 0, 0, 0 \}, kFWAudiophileRates, kFWAudiophileRates,\s*\{(.*?)\}\s*\};", ap_t, re.S)
    ap_cnt = tuple(int(x) for x in m.group(1, 2, 3, 4, 5)); ap_groups = [tuple(int(v) for v in g.split(",")) for g in re.findall(r"\{ ([\d, ]+) \}", m.group(6))]
    ap_lat = {int(k): int(v, 16) for k, v in re.findall(r"case (\d): return (0x[0-9a-f]+);", ap_t)}
    ap_lat_def = int(re.search(r"default: return (0x[0-9a-f]+);", ap_t).group(1), 16)
    if os.path.exists(slice):
        rr = [struct.unpack_from("<II", b, 0x2f720 + 8 * i)[0] for i in range(5)]
        check(ap_rates == rr, "Audiophile rates equal kSupportedSampleRatesArray @0x2f720 %s" % rr)
        w = struct.unpack_from("<6I3Q2Q20I", b, 0x4aaa0)
        check(ap_cnt == w[:5] and w[9] == w[10] == 0x2f720 and ap_groups == [tuple(w[11 + 4 * i: 15 + 4 * i]) for i in range(5)],
              "Audiophile channel row equals sDeviceChannels @0x4aaa0 (in/inS/out/outS/m %s, groups %s)" % (w[:5], ap_groups))
    dec = os.path.join(ROOT, "..", "docs/system-map/A1-ghidra/ghidra-exports/decomp/FWAudiophileDevice__GetRoundTripLatencyForFDF.c")
    if os.path.exists(dec):
        d = open(dec).read()
        ref = {int(k): int(v, 16) for v, k in re.findall(r"return (0x[0-9a-f]+);\s*\}\s*if \(param_2 (?:==|!=) (\d)\)", d)}
        check(ap_lat == {2: 0xc4, 3: 0x14a, 4: 0x162} and ap_lat_def == 0xb6 and "0xc4" in d and "0x14a" in d and "0x162" in d and "0xb6" in d,
              "Audiophile latency table equals the decompile (2:0xc4 3:0x14a 4:0x162 else 0xb6)")
    first = int(re.search(r"GetFirstSWReturnPeakMeterIndex\(\) const APPLE_KEXT_OVERRIDE \{ return (0x[0-9a-f]+)", ap_h).group(1), 16)
    hw = 14                                                  # ReadHSCIData: quadlets 0-13 = 4 inputs + 6 outputs + 2 phones + 2 aux (Q-AP0b §2)
    check(first == hw and first + ap_cnt[2] <= peak_count,
          "Audiophile meter page: hardware slots 0-%d, SW returns %d-%d (= %d out channels), inside the %d-slot page" % (hw - 1, first, first + ap_cnt[2] - 1, ap_cnt[2], peak_count))
    check(re.search(r"<key>FireWire Product Name</key>\s*<string>FW Audiophile</string>", plist) is not None and "com_openaudio_FWAudiophileDevice" in plist,
          "Audiophile personality: product name \"FW Audiophile\" -> com_openaudio_FWAudiophileDevice (reference personality)")
    oap, _, iap, _ = profile_counts("control-panel/Sources/Models/FWAudiophile/AudiophileProfile.swift")
    check(iap + oap == first and iap == ap_cnt[0], "Audiophile panel profile: inputs %d + outputs %d = kext first SW-return slot %d; inputs = kext channel row %d" % (iap, oap, first, ap_cnt[0]))

# ---- FireWire 410 module (plan FW410 v2 M-2/M-9; Q-410-0 §2-3, R135/R136) -------------------------------------------------------------
f4_dir = os.path.join(ROOT, "kext/src/models/FW410")
if os.path.isdir(f4_dir):
    f4_t = read("kext/src/models/FW410/FW410Tables.h"); f4_h = read("kext/src/models/FW410/FW410Device.h"); f4_s = read("kext/src/models/FW410/FW410Status.h")
    f4_rates = [int(x) for x in re.findall(r"\{ (\d+), 0 \}", f4_t.split("kFW410Rates[7] = {")[1].split("};")[0])]
    def f4_row(name):
        m = re.search(name + r" = \{\s*(\d+), (\d+), (\d+), (\d+), (\d+), 0, \{ ([^}]*) \}, kFW410Rates, kFW410Rates,\s*\{(.*?)\}\s*\};", f4_t, re.S)
        return (tuple(int(x) for x in m.group(1, 2, 3, 4, 5)), [x.strip() for x in m.group(6).split(",")],
                [tuple(int(v) for v in g.split(",")) for g in re.findall(r"\{ ([\d, ]+) \}", m.group(7))])
    def f4_arr(name):
        return [int(x) for x in re.search(name + r"\[\d+\] *= \{ ([\d, ]+) \};", f4_t).group(1).split(",")]
    lo, lo_maps, lo_groups = f4_row("kFW410Channels"); hi, hi_maps, hi_groups = f4_row("kFW410Channels192")
    f4_lat = {int(k): int(v, 16) for k, v in re.findall(r"case (\d): return (0x[0-9a-f]+);", f4_t)}
    if os.path.exists(slice):
        rr = [struct.unpack_from("<II", b, 0x2fa10 + 8 * i)[0] for i in range(7)]
        check(f4_rates == rr, "FW410 rates equal kSupportedInputRatesArray @0x2fa10 %s" % rr)
        for addr, cnt, maps, groups, label in ((0x4ae20, lo, lo_maps, lo_groups, "sDeviceChannels"), (0x4ad40, hi, hi_maps, hi_groups, "sDeviceChannels192")):
            w = struct.unpack_from("<6I3Q2Q20I", b, addr)
            ok = cnt == w[:5] and w[9] == w[10] == 0x2fa10 and groups == [tuple(w[11 + 4 * i: 15 + 4 * i]) for i in range(5)]
            ok = ok and [m != "0" for m in maps] == [x != 0 for x in w[6:9]]
            check(ok, "FW410 channel row equals %s @0x%x (in/inS/out/outS/m %s, maps %s, groups %s)" % (label, addr, w[:5], [hex(x) for x in w[6:9]], groups))
        im = list(struct.unpack_from("<2I", b, 0x2f9e0)); om = list(struct.unpack_from("<8I", b, 0x2f9f0))
        check(f4_arr("kFW410InputMap192") == im and f4_arr("kFW410OutputMap192") == om, "FW410 192 kHz maps equal @0x2f9e0 %s / @0x2f9f0 %s" % (im, om))
    check(f4_lat == {2: 0xa0, 3: 0xfe, 4: 0xf5, 5: 0x1b8, 6: 0x1e0} and "default: return 0x96" in f4_t,
          "FW410 latency table equals the decompile (2:0xa0 3:0xfe 4:0xf5 5:0x1b8 6:0x1e0 else 0x96)")
    first = int(re.search(r"GetFirstSWReturnPeakMeterIndex\(\) const APPLE_KEXT_OVERRIDE \{ return (\d+)", f4_h).group(1))
    hwout = int(re.search(r"kFW410FirstHwOutSlot = (\d+)", f4_s).group(1)); nout = int(re.search(r"kFW410HwOutPeaks = (\d+)", f4_s).group(1))
    check(first == 4 and first + lo[2] == hwout and hwout + nout <= peak_count,
          "FW410 meter page: hardware inputs 0-3, SW returns %d-%d (= %d out channels), hardware outputs %d-%d, inside the %d-slot page" % (first, first + lo[2] - 1, lo[2], hwout, hwout + nout - 1, peak_count))
    check(re.search(r"<key>FireWire Product Name</key>\s*<string>FW 410</string>", plist) is not None and "com_openaudio_FW410Device" in plist,
          "FW410 personality: product name \"FW 410\" -> com_openaudio_FW410Device (reference personality)")
    o4, m4, i4, h4 = profile_counts("control-panel/Sources/Models/FW410/FW410Profile.swift")
    pf = read("control-panel/Sources/Models/FW410/FW410Profile.swift")
    check("var peakSWReturnsFirst: Bool { return true }" in pf and i4 == first == lo[0] and i4 + (m4 - i4) == hwout and m4 - i4 == lo[2] and o4 == nout,
          "FW410 panel profile: inputs %d = kext first SW-return slot %d; SW returns %d (mixer %d - inputs) end at the kext's first output slot %d; outputs %d = kext hardware output peaks" % (i4, first, m4 - i4, m4, hwout, o4))

# ---- modular / generic first (plan v4 AP-2.5 g, h) ------------------------------------------------------------------------------------
# base/ must not include a model header, and holds no model name in code (comments are provenance and stay). The one exception is the
# reference's own device table kDeviceSpecs in FWMetaNub.cpp (@0x46450 lists every model by name in the base nub).
MODEL = re.compile(r"1814|Audiophile|FW ?410|Ozonic|FW ?Solo|NRV-?10|Lightbridge|ProjectMix")
leaks = []
for path in sorted(glob.glob(os.path.join(ROOT, "kext/src/base/*.[ch]*"))):
    text = open(path).read()
    if path.endswith("FWMetaNub.cpp"): text = re.sub(r"kDeviceSpecs\[\] = \{.*?\};", "", text, flags=re.S)
    for n, line in enumerate(text.split("\n"), 1):
        code = re.sub(r"//.*", "", line)
        inc = re.match(r'\s*#include "(\w+)\.h"', code)
        if inc and os.path.exists(os.path.join(ROOT, "kext/src/models")) and glob.glob(os.path.join(ROOT, "kext/src/models/*/%s.h" % inc.group(1))):
            leaks.append("%s:%d includes model header %s.h" % (os.path.basename(path), n, inc.group(1)))
        for lit in re.findall(r'"([^"\n]*)"', code):
            if MODEL.search(lit): leaks.append("%s:%d literal \"%s\"" % (os.path.basename(path), n, lit))
        if re.search(r"\bcom_(openaudio|m_audio)_\w*(1814|Audiophile|410|Ozonic|Solo|NRV10|Lightbridge|ProjectMix)\w*", code):
            leaks.append("%s:%d model class in code" % (os.path.basename(path), n))
check(not leaks, "base/ is model-free (no model include, literal or class in code): %s" % ("; ".join(leaks) or "clean"))
models = sorted(os.path.basename(p) for p in glob.glob(os.path.join(ROOT, "kext/src/models/*")))
check(bool(models), "model modules present: %s" % ", ".join(models))

print("\n%s: %d failure(s)" % ("CONFORMANCE FAILED" if fails else "CONFORMANT", fails))
sys.exit(1 if fails else 0)
