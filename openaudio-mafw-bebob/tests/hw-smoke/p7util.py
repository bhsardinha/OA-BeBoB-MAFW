#!/usr/bin/python
# Helpers for tests/hw-smoke/phase7.sh (runs with the system Python 2.7 of 10.13 and with Python 3). Reads the registry through `ioreg -a`
# (XML), so property values with commas or newlines survive. Read-only, except that `mkset` writes a plist file (applied by `maprop setxml`).
#   p7util.py units                       FireWire bus nodes: product, GUID, Model_ID (one line each)
#   p7util.py bus-watch                   print the node list with a timestamp whenever it changes (0.5 s poll; bootloader -> runtime)
#   p7util.py trace-watch                 print every new MADebugTrace line of every unit with a timestamp (1 s poll; the ring holds 16)
#   p7util.py ma-watch MAPROP SECONDS     print every MA* key change of the unit MA_CLASS selects (front-panel checks, AP-7.7)
#   p7util.py getkey FILE KEY             print KEY of a `maprop allxml` dump
#   p7util.py mkset FILE KEY OUT I=V ...  write OUT = {KEY: FILE[KEY] with element I set to V}, for `maprop setxml OUT`
import plistlib, subprocess, sys, time

def ioreg(cls, depth=None):
    cmd = ["ioreg", "-a", "-r", "-c", cls] + (["-d", str(depth)] if depth else [])
    out = subprocess.Popen(cmd, stdout=subprocess.PIPE).communicate()[0]
    if not out.strip():
        return []
    return plistlib.loads(out) if hasattr(plistlib, "loads") else plistlib.readPlistFromString(out)

def load(path):
    with open(path, "rb") as f:
        return plistlib.load(f) if hasattr(plistlib, "load") else plistlib.readPlist(f)

def units():
    rows = []
    for d in ioreg("IOFireWireDevice", 1):
        rows.append("%s GUID=%x Model_ID=0x%x" % (d.get("FireWire Product Name", "?"), d.get("GUID", 0), d.get("Model_ID", 0)))
    return sorted(rows)

def now():
    t = time.time()
    return time.strftime("%H:%M:%S", time.localtime(t)) + ".%03d" % int((t % 1) * 1000)

def bus_watch():
    prev = None
    while True:
        cur = units()
        if cur != prev:
            print(now()); print("\n".join(cur) if cur else "(no units)"); sys.stdout.flush(); prev = cur
        time.sleep(0.5)

def new_lines(prev, cur):
    # the ring drops its oldest entry when full: the new lines are what follows the longest overlap of prev's tail with cur's head
    for k in range(min(len(prev), len(cur)), 0, -1):
        if prev[-k:] == cur[:k]:
            return cur[k:]
    return cur

def trace_watch():
    rings = {}
    while True:
        for d in ioreg("com_openaudio_FWAVCAudioDevice", 1):
            name = "%s %x" % (d.get("IOClass", "?"), d.get("GUID", 0))
            cur = list(d.get("MADebugTrace", []))
            for line in new_lines(rings.get(name, []), cur):
                print("%s %s: %s" % (now(), name, line)); sys.stdout.flush()
            rings[name] = cur
        time.sleep(1)

def ma_watch(maprop, secs):
    # every MA* key change of one unit (MA_CLASS selects it), via `maprop allxml` (MAMetaAllProperties) ~3 times a second
    prev, end = None, time.time() + secs
    while time.time() < end:
        out = subprocess.Popen([maprop, "allxml"], stdout=subprocess.PIPE).communicate()[0]
        cur = plistlib.loads(out) if hasattr(plistlib, "loads") else plistlib.readPlistFromString(out)
        if prev is not None:
            for k in sorted(cur):
                if cur[k] != prev.get(k):
                    print("%s %-26s %s -> %s" % (now(), k, prev.get(k), cur[k])); sys.stdout.flush()
        prev = cur
        time.sleep(0.3)

def main(a):
    if a[:1] == ["units"]:
        print("\n".join(units()))
    elif a[:1] == ["bus-watch"]:
        bus_watch()
    elif a[:1] == ["trace-watch"]:
        trace_watch()
    elif a[:1] == ["ma-watch"] and len(a) == 3:
        ma_watch(a[1], float(a[2]))
    elif a[:1] == ["getkey"] and len(a) == 3:
        print(load(a[1]).get(a[2]))
    elif a[:1] == ["mkset"] and len(a) >= 5:
        v = list(load(a[1])[a[2]])
        for iv in a[4:]:
            i, x = iv.split("=")
            v[int(i)] = int(x)
        with open(a[3], "wb") as f:
            (plistlib.dump if hasattr(plistlib, "dump") else plistlib.writePlist)({a[2]: v}, f)
        print("%s -> %s" % (a[2], v))
    else:
        sys.stderr.write(open(__file__).read().split("import")[0]); return 2
    return 0

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
