#!/bin/bash
# Install kext + helper daemon on the build PC. ONLY after scripts/stage-kext.sh succeeded (kextutil -t -n on a
# /tmp copy): a bad kext in /Library/Extensions causes an autoload boot loop (feedback_stage_kext_before_autoload).
# Usage: scripts/install.sh   (run from the repo copy that holds kext/build and firmware-helper/build)
set -euo pipefail
cd "$(dirname "$0")/.."
KEXT=kext/build/OpenAudioBeBoBMAFW.kext
HELPER=firmware-helper/build/oa-fwhelper
PLIST=firmware-helper/com.openaudio.bebobmafw.helper.plist
[ -d "$KEXT" ] && [ -x "$HELPER" ] || { echo "build first"; exit 1; }
[ -d /tmp/OpenAudioBeBoBMAFW.kext ] || { echo "run scripts/stage-kext.sh first"; exit 1; }
# Stop the helper FIRST (2026-10-17): it holds a user client on the device until it sees the device terminate, and kextunload refuses
# (exit 3, kext in use) while that reference exists. The retry covers the short window in which another client is still releasing.
sudo launchctl unload /Library/LaunchDaemons/com.openaudio.bebobmafw.helper.plist 2>/dev/null || true
# 0.1.0 generation (OpenAudio FW1814): same com_openaudio_* C++ classes, so it can never be loaded together with this driver.
OLD_KEXT_ID=com.openaudio.OpenAudioFW1814; OLD_KEXT=/Library/Extensions/OpenAudioFW1814.kext
OLD_HELPER=com.openaudio.fw1814.helper; OLD_APP="/Applications/OpenAudio FW1814.app"
# Replace the 0.1.0 generation (plan v3 §0/§7): stop its helper and panel, unload and remove its kext and helper files.
sudo launchctl unload /Library/LaunchDaemons/$OLD_HELPER.plist 2>/dev/null || true
pkill -f "$OLD_APP" 2>/dev/null || true
# (2026-10-21: `|| true` -- with set -e a failing kextunload, i.e. the 0.1.0 kext not loaded, ended the script silently before the copy)
for try in 1 2 3 4; do sudo kextunload -b $OLD_KEXT_ID 2>/dev/null || true; kextstat | grep -q $OLD_KEXT_ID || break; sleep 3; done
sudo rm -rf $OLD_KEXT /Library/LaunchDaemons/$OLD_HELPER.plist /Library/PrivilegedHelperTools/$OLD_HELPER
for try in 1 2 3 4; do sudo kextunload -b com.openaudio.OpenAudioBeBoBMAFW 2>/dev/null && break || sleep 3; done
sudo rm -rf /Library/Extensions/OpenAudioBeBoBMAFW.kext
sudo cp -R "$KEXT" /Library/Extensions/OpenAudioBeBoBMAFW.kext
sudo chown -R root:wheel /Library/Extensions/OpenAudioBeBoBMAFW.kext
sudo chmod -R go-w /Library/Extensions/OpenAudioBeBoBMAFW.kext
sudo mkdir -p /Library/PrivilegedHelperTools
sudo cp "$HELPER" /Library/PrivilegedHelperTools/com.openaudio.bebobmafw.helper
sudo chown root:wheel /Library/PrivilegedHelperTools/com.openaudio.bebobmafw.helper
sudo chmod 755 /Library/PrivilegedHelperTools/com.openaudio.bebobmafw.helper
sudo cp "$PLIST" /Library/LaunchDaemons/com.openaudio.bebobmafw.helper.plist
sudo chown root:wheel /Library/LaunchDaemons/com.openaudio.bebobmafw.helper.plist
sudo chmod 644 /Library/LaunchDaemons/com.openaudio.bebobmafw.helper.plist
sudo kextcache -system-caches
echo "installed. Reboot to test autoload (kext loads on first matching device; the helper starts at boot)."
