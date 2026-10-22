#!/bin/bash
# Modern edition (macOS 11-15, Intel) developer install, BROAD_COMPATIBILITY_PLAN_v2 §11a. Installs the 0.2.2 package payload (helper, MIDI plug-in,
# icons: unchanged user space), replaces the kext with the BAND=modern build (kext/build/modern, 15.x SDK) and, when present, the panel with the
# Modern panel (control-panel/build/modern: `make -C control-panel app BAND=modern` on the build PC, copied back; About tab names the edition).
# Why not the 10.13 kext: it imports IOAudioEngine::_RESERVEDIOAudioEngine15/16, which IOAudioFamily 540 no longer defines (R125, E-17).
# Gate (replaces `kextutil -t -n`, which kmutil rejects): every import of the kext must resolve on this system (kmutil libraries).
# Loading: macOS 11+ links the kext into the auxiliary kernel collection; kmutil load stages it, you approve it in System Settings >
# Privacy & Security, then RESTART. Recovery if the Mac misbehaves after that: boot Safe Mode (hold Shift) and run scripts/uninstall.sh.
# Usage: scripts/install-modern.sh [--check]     (--check = build + gate only, installs nothing, needs no sudo)
set -euo pipefail
cd "$(dirname "$0")/.."
PKG=dist/OA-BeBoB-MAFW-0.2.2.pkg
PANEL="control-panel/build/modern/OA BeBoB MAFW.app"
KEXT=kext/build/modern/OpenAudioBeBoBMAFW.kext
BIN=$KEXT/Contents/MacOS/OpenAudioBeBoBMAFW
[ "$(sw_vers -productVersion | cut -d. -f1)" -ge 11 ] || { echo "Modern edition is for macOS 11+"; exit 1; }
[ "$(uname -m)" = x86_64 ] || { echo "Intel only for now (Apple Silicon is S2)"; exit 1; }
make -C kext BAND=modern >/dev/null
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
nm -u "$BIN" | sort -u > "$TMP/u"
kmutil libraries -p "$KEXT" -a x86_64 -z 2>&1 | grep ' in /' | awk '{print $1}' | sort -u > "$TMP/r"
MISSING=$(comm -23 "$TMP/u" "$TMP/r")
if [ -n "$MISSING" ]; then echo "gate FAILED, unresolved imports:"; echo "$MISSING" | c++filt; exit 1; fi
echo "gate OK: $(wc -l < "$TMP/u" | tr -d ' ') of $(wc -l < "$TMP/u" | tr -d ' ') imports resolve"
[ "${1:-}" = --check ] && exit 0

pkgutil --expand-full "$PKG" "$TMP/pkg"
PAYLOAD="$TMP/pkg/component.pkg/Payload"
sudo launchctl bootout system/com.openaudio.bebobmafw.helper 2>/dev/null || true
sudo rm -rf /Library/Extensions/OpenAudioBeBoBMAFW.kext
sudo ditto "$PAYLOAD" /
sudo rm -rf /Library/Extensions/OpenAudioBeBoBMAFW.kext
sudo ditto "$KEXT" /Library/Extensions/OpenAudioBeBoBMAFW.kext
if [ -d "$PANEL" ]; then
    pkill -f "/Applications/OA BeBoB MAFW.app/Contents/MacOS/" || true
    sudo rm -rf "/Applications/OA BeBoB MAFW.app"
    sudo ditto "$PANEL" "/Applications/OA BeBoB MAFW.app"
    echo "panel: Modern build ($(/usr/libexec/PlistBuddy -c 'Print :OAEdition' "$PANEL/Contents/Info.plist"))"
fi
sudo chown -R root:wheel /Library/Extensions/OpenAudioBeBoBMAFW.kext "/Library/Audio/MIDI Drivers/MAudioFireWireMIDI.plugin" \
    /Library/PrivilegedHelperTools/com.openaudio.bebobmafw.helper /Library/LaunchDaemons/com.openaudio.bebobmafw.helper.plist \
    "/Library/Application Support/M-Audio/DeviceIcons"
sudo chmod -R go-w /Library/Extensions/OpenAudioBeBoBMAFW.kext
sudo launchctl bootstrap system /Library/LaunchDaemons/com.openaudio.bebobmafw.helper.plist 2>/dev/null || true
sudo killall MIDIServer 2>/dev/null || true
# Stage the kext for the auxiliary collection; macOS asks for approval and a restart.
sudo kmutil load -p /Library/Extensions/OpenAudioBeBoBMAFW.kext || true
echo
echo "Next: System Settings > Privacy & Security > Allow (OpenAudio), then restart. Plug the interface in after the restart."
