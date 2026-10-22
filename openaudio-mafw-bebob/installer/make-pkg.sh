#!/bin/bash
# Dev-Mac driver for IN.5: sync the tree, ship the reference assets (MIDI plug-in, icons from _extracted/) to the build PC, build the .pkg there,
# fetch it to dist/. Usage: installer/make-pkg.sh [classic|modern]   (edition, default classic; BROAD_COMPATIBILITY_PLAN_v2 §11a, R128)
# Modern: the kext is built HERE (15.x SDK, BAND=modern), must link on this Mac's kernel (every nm -u import found by kmutil libraries, as the
# package's postinstall checks on the target) and goes to the build PC with that import list; the build PC builds the helper and the panel.
set -euo pipefail
HOST=${BUILDPC:-bernardosardinha@192.168.100.20}
cd "$(dirname "$0")/.."
REPO=$(cd .. && pwd)
BAND=${1:-classic}
case $BAND in classic|modern) ;; *) echo "usage: installer/make-pkg.sh [classic|modern]"; exit 1;; esac
scripts/sync-to-buildpc.sh
if [ "$BAND" = modern ]; then
  K=kext/build/modern/OpenAudioBeBoBMAFW.kext
  make -C kext BAND=modern 2>&1 | grep -v "was built for newer macOS version" || true      # ld's libkmod 15.2 notes are expected (kext/Makefile)
  LC_ALL=C nm -u "$K/Contents/MacOS/OpenAudioBeBoBMAFW" | LC_ALL=C sort -u > kext/build/modern/imports.txt
  MISSING=$(LC_ALL=C comm -23 kext/build/modern/imports.txt <(kmutil libraries -p "$K" -a x86_64 -z 2>/dev/null | awk '/ in \//{print $1}' | LC_ALL=C sort -u))
  [ -z "$MISSING" ] || { echo "Modern kext does not link on this Mac:"; echo "$MISSING" | c++filt; exit 1; }
  echo "Modern kext: $(wc -l < kext/build/modern/imports.txt | tr -d ' ') imports, all found by kmutil"
  ssh "$HOST" 'rm -rf ~/oa-build-mafw/kext/build/modern && mkdir -p ~/oa-build-mafw/kext/build/modern'
  rsync -a "$K" kext/build/modern/imports.txt "$HOST":oa-build-mafw/kext/build/modern/
fi
rsync -a installer "$HOST":oa-build-mafw/
ssh "$HOST" 'mkdir -p ~/oa-build-mafw/installer/assets'
# M-Audio's CoreMIDI plug-in is NOT part of the public repository: copy it from the official M-Audio FireWire 1.11.2 installer into installer/assets/
# (the device icons ship in installer/assets/DeviceIcons). The private tree keeps them under _extracted/.
PLUGIN=installer/assets/MAudioFireWireMIDI.plugin; [ -d "$PLUGIN" ] || PLUGIN="$REPO/_extracted/midi/MAudioFireWireMIDI.plugin"
ICONS=installer/assets/DeviceIcons;               [ -d "$ICONS" ]  || ICONS="$REPO/_extracted/icons"
[ -d "$PLUGIN" ] || { echo "missing MAudioFireWireMIDI.plugin: take it from the official M-Audio FireWire 1.11.2 installer and put it in openaudio-fw1814/installer/assets/"; exit 1; }
[ -d "$ICONS" ]  || { echo "missing device icons (installer/assets/DeviceIcons)"; exit 1; }
rsync -a --delete "$PLUGIN" "$HOST":oa-build-mafw/installer/assets/
rsync -a --delete "$ICONS"/ "$HOST":oa-build-mafw/installer/assets/DeviceIcons/
ssh "$HOST" "cd ~/oa-build-mafw && BAND=$BAND installer/build-pkg.sh"
mkdir -p dist
V=$(cat VERSION)
scp "$HOST":"oa-build-mafw/installer/build/OA-BeBoB-MAFW-$V-$BAND.pkg" dist/
ls -l "dist/OA-BeBoB-MAFW-$V-$BAND.pkg"
