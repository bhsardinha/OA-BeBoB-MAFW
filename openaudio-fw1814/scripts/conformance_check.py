#!/usr/bin/env python3
"""Panel <-> kext <-> plist conformance (BUILD-PLAN §0, Build row IN.7). Static: reads the sources, needs no hardware or toolchain.

Checks (each prints OK/FAIL, exit status 1 if any FAIL):
  1. every MA* key of the panel catalog is registered by the kext, with a compatible kind (array / number / 64-bit / function)
  2. array element counts of the panel catalog equal the kext's counts (group templates in FWMixer.h, counts in the AddArray calls)
  3. the panel's match class equals the personality IOClass, the personality carries IOUserClientClass, the helper matches the nub class
  4. the panel's meter-page map (52 slots = 18 + 20 + 14) equals the kext's peak constants and the user-client memory type
  5. the panel's per-rate channel counts equal the kext's DeviceChannels tables (FWRates.h)
  6. CoreAudio controls are created by FWSettingsLevels::Init
Usage: scripts/conformance_check.py   (from anywhere; paths are relative to the repo's openaudio-fw1814/ directory)
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
mixer_h = read("kext/src/FWMixer.h")
cnt = {}
for n, names in re.findall(r"FWConcreteChannelGroup<(\d+)>\s+([^;]+);", mixer_h):
    for nm in names.split(","): cnt[nm.strip()] = int(n)
group_count = {"SWReturn": cnt["fSWReturn"], "Output": cnt["fOutput"], "Input": cnt["fInput"], "Headphone": cnt["fHeadphone"],
               "AuxOutput": cnt["fAuxOut"], "AuxSend": cnt["fAuxSend"]}
sym = {"swReturn": cnt["fSWReturn"], "output": cnt["fOutput"], "input": cnt["fInput"], "headphone": cnt["fHeadphone"],
       "auxOut": cnt["fAuxOut"], "auxSend": cnt["fAuxSend"]}
src = "".join(read(os.path.relpath(p, ROOT)) for p in sorted(glob.glob(os.path.join(ROOT, "kext/src/*.cpp"))))
mixer_cpp = read("kext/src/FWMixer.cpp")
n_monitor = (cnt["fOutput"] >> 1) + 1                     # FWMixer.cpp: fAux->Init(..., (fOutput.Count() >> 1) + 1)
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
check(re.search(r"<key>IOClass</key>\s*<string>%s</string>" % panel_class, plist) is not None, "panel match class %s is a personality IOClass" % panel_class)
m = re.search(r"<key>FW1814Device</key>.*?</dict>", plist, re.S)
check(m is not None and "IOUserClientClass" in m.group(0), "FW1814Device personality carries IOUserClientClass")
helper_class = re.search(r'kNubClass = "(\w+)"', read("firmware-helper/main.c")).group(1)
check(re.search(r"<key>IOClass</key>\s*<string>%s</string>" % helper_class, plist) is not None, "helper nub class %s is a personality IOClass" % helper_class)
for key in ("MAFirmwareStart", "MAFirmwareInfo"):
    check(key in literals, "helper/panel key %s exists in the kext" % key)

# ---- meter page --------------------------------------------------------------------------------------------------------------------
chain_h = read("kext/src/FWAudioDeviceChain.h")
peak_count = int(re.search(r"GetPeakLevelCount\(\) const \{ return (0x[0-9a-f]+)", chain_h).group(1), 16)
sw_first = int(re.search(r"GetFirstSWReturnPeakMeterIndex\(\) const \{ return (0x[0-9a-f]+)", chain_h).group(1), 16)
sw_count = int(re.search(r"GetSWReturnPeakMeterCount\(\) const \{ return (0x[0-9a-f]+)", chain_h).group(1), 16)
bridge = read("control-panel/Sources/IOKitBridge.swift")
panel_peak = int(re.search(r"static let peakCount = (\d+)", bridge).group(1))
pin = int(re.search(r"static func swReturn\(_ k: Int\) -> Int \{ return (\d+) \+", bridge).group(1))
pout = int(re.search(r"static func output\(_ channel: Int\) -> Int \{ return (\d+) \+", bridge).group(1))
check(panel_peak == peak_count, "meter page slots: panel %d, kext GetPeakLevelCount %d" % (panel_peak, peak_count))
check(pin == sw_first and sw_first + sw_count == peak_count, "SW-return slots start at %d (kext %d), %d slots to the end of the page" % (pin, sw_first, sw_count))
check(pout == cnt["fInput"], "output slots start after the %d input slots (panel %d)" % (cnt["fInput"], pout))
uc = read("kext/src/FW1814UserClient.cpp")
check("type == 1" in uc and "meterMemory" in uc, "user client serves the meter page as clientMemoryForType(1); panel maps type 1")
check("IOConnectMapMemory64(connection, 1," in bridge, "panel maps memory type 1")

# ---- per-rate channel counts ---------------------------------------------------------------------------------------------------------
rates_h = read("kext/src/FWRates.h")
tables = {n: (int(i), int(o)) for n, i, o in re.findall(r"FWDeviceChannels (kF\w+)\s*=\s*FWDC_HEAD\((\d+),\s*(\d+)\)", rates_h)}
analog_in, analog_out, spdif, adat = 8, 4, 2, 8
exp = {   # (kext table, panel expectation from ChannelLayout: analog + digital)
    "kFWSPDIF44_96": (analog_in + spdif, analog_out + spdif),
    "kFWADAT44_48": (analog_in + adat, analog_out + adat),
    "kFWADAT88_96": (analog_in + adat // 2, analog_out + adat // 2),
    "kFW176_192": (2, 4),
}
for t, (i, o) in exp.items():
    check(tables.get(t) == (i, o), "FWRates %-14s in/out kext %s, panel layout %s" % (t, tables.get(t), (i, o)))
layout = read("control-panel/Sources/ChannelLayout.swift")
check("rateClass == .quad { return s }" in layout and "adatHalf" in layout, "ChannelLayout implements the 176.4/192 kHz analog-only and the 88.2/96 kHz ADAT-half rules")

# ---- CoreAudio controls -------------------------------------------------------------------------------------------------------------
settings = read("kext/src/FWSettings.cpp")
for needle in ("EstablishVolumeControl(eng, kOutp, 0, 0", "EstablishVolumeControls(eng, kOutp, 1", "EstablishMuteControls(eng, kOutp, 1",
               "EstablishVolumeControls(eng, kInpt, 1", "EstablishMuteControls(eng, kInpt, 1", "EstablishMuteControl(eng, kIOAudioControlUsageOutput, 0, 0)"):
    check(needle in settings, "FWSettingsLevels::Init creates: %s" % needle)

print("\n%s: %d failure(s)" % ("CONFORMANCE FAILED" if fails else "CONFORMANT", fails))
sys.exit(1 if fails else 0)
