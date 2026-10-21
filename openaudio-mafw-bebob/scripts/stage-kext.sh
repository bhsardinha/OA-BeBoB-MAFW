#!/bin/bash
# Stage -> validate -> (never auto-install). Mandatory per feedback_stage_kext_before_autoload.md:
# a bad kext in /Library/Extensions boot-loops the build PC, so this script stops at a validated /tmp copy.
# Usage (on the build PC): scripts/stage-kext.sh [path-to-built-kext]
set -euo pipefail
SRC=${1:-kext/build/OpenAudioBeBoBMAFW.kext}
DST=/tmp/OpenAudioBeBoBMAFW.kext
[ -d "$SRC" ] || { echo "missing $SRC (run: make -C kext)"; exit 1; }
sudo rm -rf "$DST"
sudo cp -R "$SRC" "$DST"
sudo chown -R root:wheel "$DST"
sudo chmod -R go-w "$DST"
sudo kextutil -t -n "$DST"
echo "STAGED + kextutil -t -n OK: $DST   (load with: sudo kextutil $DST; nothing was copied to /Library/Extensions)"
