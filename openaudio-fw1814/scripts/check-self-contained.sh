#!/bin/bash
# Guard for the installer: every Mach-O in the staged payload (and in the uninstaller app) may depend only on macOS itself (/System/Library, /usr/lib) or on
# libraries inside its own bundle (@rpath resolving to Contents/Frameworks), and no rpath may point outside the bundle (the first release pointed into Xcode and
# crashed on a clean Mac). Usage: scripts/check-self-contained.sh <dir> [<dir>...]      exit 1 = a dependency would be missing on a clean 10.13.6 Mac.
set -uo pipefail
bad=0
for ROOT in "$@"; do
  while IFS= read -r -d '' f; do
    file -b "$f" | grep -q "Mach-O" || continue
    case "$f" in *.kext/Contents/MacOS/*) continue;; esac                       # kexts link against kernel KPIs (checked by kextutil -t -n), not dylibs
    # dylib dependencies
    for dep in $(otool -L "$f" | tail -n +2 | awk '{print $1}'); do
      case "$dep" in
        /System/Library/*|/usr/lib/*) ;;
        @rpath/*) lib=${dep#@rpath/}; app=${f%%.app/*}.app
                  [ -f "$app/Contents/Frameworks/$lib" ] || { echo "NOT SELF-CONTAINED: $f needs $dep which is not in $app/Contents/Frameworks"; bad=1; } ;;
        *) echo "NOT SELF-CONTAINED: $f depends on $dep"; bad=1 ;;
      esac
    done
    # rpaths
    for rp in $(otool -l "$f" | awk '/cmd LC_RPATH/{g=1} g && /^ +path /{print $2; g=0}'); do
      case "$rp" in @executable_path/../Frameworks|@loader_path|@executable_path) ;; *) echo "BAD RPATH: $f has rpath $rp"; bad=1 ;; esac
    done
  done < <(find "$ROOT" -type f -print0)
done
[ $bad = 0 ] && echo "self-contained check passed"
exit $bad
