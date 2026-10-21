#!/usr/bin/env python3
"""factory-diff.py -- compare the driver's factory state with the official driver's (plan v4 AP-3.6 / AP-7 acceptance).

Usage (on the test Mac, after MAMetaResetToFactorySettings):
    tools/maprop allxml > now.plist
    tools/factory-diff.py now.plist REFERENCE.plist GUID
REFERENCE.plist = the official driver's settings file after its factory reset, e.g.
    docs/system-map/B5-sl-dtrace-truth/captures/92-sl-audiophile-controls-rates-reset/reset/device-settings-after-reset.plist
GUID = the reference file's top-level key (Audiophile D6C0300211D54). Keys that describe the session rather than the factory state
(rate, lock, bandwidth) are skipped. Exit status 1 when a mixer key differs.
"""
import plistlib, sys

SKIP = {"MASampleRate", "MARateLocked", "MABytesPerCycle", "MAEnableAggregateEngine", "MAEnableStereoPairEngines"}
load = (lambda p: plistlib.load(open(p, "rb"))) if hasattr(plistlib, "load") else plistlib.readPlist   # Python 3, or 10.13's Python 2.7
now = load(sys.argv[1])
ref = load(sys.argv[2])[sys.argv[3]]
diff = 0
for k in sorted(ref):
    if k in SKIP: continue
    a, b = now.get(k), ref[k]
    same = a == b
    print("%s %-28s driver %-40s reference %s" % ("OK  " if same else "DIFF", k, a, b))
    diff += not same
missing = sorted(k for k in now if k.startswith("MA") and k not in ref and k not in SKIP)
print("\nkeys only in the driver: %s" % (", ".join(missing) or "none"))
print("%s: %d differing key(s)" % ("FACTORY STATE EQUAL" if not diff else "FACTORY STATE DIFFERS", diff))
sys.exit(1 if diff else 0)
