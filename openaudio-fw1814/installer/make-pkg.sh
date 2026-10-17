#!/bin/bash
# Dev-Mac driver for IN.5: sync the tree, ship the reference assets (MIDI plug-in, icons from _extracted/) to the build PC, build the .pkg there,
# fetch it to dist/. Usage: installer/make-pkg.sh
set -euo pipefail
HOST=${BUILDPC:-bernardosardinha@192.168.100.20}
cd "$(dirname "$0")/.."
REPO=$(cd .. && pwd)
scripts/sync-to-buildpc.sh
rsync -a installer "$HOST":oa-build/
ssh "$HOST" 'mkdir -p ~/oa-build/installer/assets'
# M-Audio's CoreMIDI plug-in is NOT part of the public repository: copy it from the official M-Audio FireWire 1.11.2 installer into installer/assets/
# (the device icons ship in installer/assets/DeviceIcons). The private tree keeps them under _extracted/.
PLUGIN=installer/assets/MAudioFireWireMIDI.plugin; [ -d "$PLUGIN" ] || PLUGIN="$REPO/_extracted/midi/MAudioFireWireMIDI.plugin"
ICONS=installer/assets/DeviceIcons;               [ -d "$ICONS" ]  || ICONS="$REPO/_extracted/icons"
[ -d "$PLUGIN" ] || { echo "missing MAudioFireWireMIDI.plugin: take it from the official M-Audio FireWire 1.11.2 installer and put it in openaudio-fw1814/installer/assets/"; exit 1; }
[ -d "$ICONS" ]  || { echo "missing device icons (installer/assets/DeviceIcons)"; exit 1; }
rsync -a --delete "$PLUGIN" "$HOST":oa-build/installer/assets/
rsync -a --delete "$ICONS"/ "$HOST":oa-build/installer/assets/DeviceIcons/
ssh "$HOST" 'cd ~/oa-build && installer/build-pkg.sh'
mkdir -p dist
scp "$HOST":'oa-build/installer/build/OpenAudioFW1814-*.pkg' dist/
ls -l dist/*.pkg
